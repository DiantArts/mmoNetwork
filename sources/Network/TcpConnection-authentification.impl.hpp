#pragma once

// ------------------------------------------------------------------ async - Server Authentification

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverAuthentification()
{
    this->serverReceiveAuthentification();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverReceiveAuthentification()
{
    ::std::this_thread::sleep_for(500ms);
    this->receiveMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            if (
                self.m_bufferIn.getTypeAsSystemType() !=
                ::network::Message<UserMessageType>::SystemType::authentification
            ) {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Authentification failed, "
                    << "unexpected message received.\n";
                return self.disconnect();
            }
            auto password{ self.m_bufferIn.template extract<::std::string>() };
            self.setUserName(self.m_bufferIn.template extract<::std::string>());
            if (!self.m_owner.onAuthentification(self.shared_from_this())) {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Authentification failed, "
                    << "onAuthentification returned false.\n";
                self.m_owner.onAuthentificationDenial(self.shared_from_this());
                self.sendAuthentificationDenial();
                return self.serverAuthentification();
            }
            ::std::cerr << "[Connection:TCP:" << self.m_id << "] Authentification successful.\n";
            self.serverSendAuthentificationAcceptance();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            if (errorCode == ::asio::error::operation_aborted) {
                ::std::cerr << "[Connection:TCP] Operation canceled\n";
            } else if (errorCode == ::asio::error::eof) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Node stopped the connection.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read authentification header failed: "
                    << errorCode.message() << ".\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read authentification body failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverSendAuthentificationAcceptance()
{
    this->sendMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            ::std::cerr << "[Connection:TCP:" << self.m_id << "] Authentification successful.\n";
            self.m_isValid = true;
            self.m_owner.onConnectionValidated(self.shared_from_this());
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send authentification acceptance header: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send authentification acceptance body: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(::network::Message<UserMessageType>{ ::network::Message<UserMessageType>::SystemType::authentificationAccepted });
}



// ------------------------------------------------------------------ async - Client Authentification
// TODO: mem error when closing the client after authentification denial

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientAuthentification()
{
    m_owner.onAuthentification(this->shared_from_this());
    this->clientSendAuthentification();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientSendAuthentification()
{
    ::std::this_thread::sleep_for(500ms);
    this->sendMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            self.clientReceiveAuthentificationAcceptance();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send authentification header: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send authentification body: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(::network::Message<UserMessageType>{ ::network::Message<UserMessageType>::SystemType::authentification, m_userName, ""s });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientReceiveAuthentificationAcceptance()
{
    this->receiveMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            if (
                self.m_bufferIn.getTypeAsSystemType() ==
                ::network::Message<UserMessageType>::SystemType::authentificationDenied
            ) {
                self.m_owner.onAuthentificationDenial(self.shared_from_this());
                self.clientSendAuthentification();
            } else if (
                    self.m_bufferIn.getTypeAsSystemType() ==
                    ::network::Message<UserMessageType>::SystemType::authentificationAccepted
                ) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Authentification successful.\n";
                self.m_isValid = true;
                self.m_owner.onConnectionValidated(self.shared_from_this());
            } else {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] invalid authentification acceptance\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            if (errorCode == ::asio::error::operation_aborted) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Operation canceled\n";
            } else if (errorCode == ::asio::error::eof) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Node stopped the connection.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:TCP:" << self.m_id
                    << "] Read authentification acception header failed: " << errorCode.message() << ".\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read authentification acception body failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}



// ------------------------------------------------------------------ async - Error Authentification

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::sendAuthentificationDenial()
{
    this->sendMessage<
        [](...){},
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Authentification:TCP:" << self.m_id
                << "] Send identificaion denied header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Authentification:TCP:" << self.m_id
                << "] Send identificaion denied body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(::network::Message<UserMessageType>{ ::network::Message<UserMessageType>::SystemType::authentificationDenied });
}
