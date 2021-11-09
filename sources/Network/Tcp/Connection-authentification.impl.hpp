#pragma once

// ------------------------------------------------------------------ async - Server Authentification

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverAuthentification()
{
    this->serverReceiveAuthentification();
}

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverReceiveAuthentification()
{
    this->receiveMessage([](::network::tcp::Connection<UserMessageType>& self){
        if (
            self.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::authentification
        ) {
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Authentification failed, "
                << "unexpected message received.\n";
            return self.disconnect();
        }
        auto password{ self.m_bufferIn.template extract<::std::string>() };
        auto userName{ self.m_bufferIn.template extract<::std::string>() };
        ::std::cout << userName << ::std::endl;
        self.setUserName(userName);
        if (!self.m_owner.onAuthentification(self.shared_from_this())) {
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Authentification failed, "
                << "onAuthentification returned false.\n";
            self.m_owner.onAuthentificationDenial(self.shared_from_this());
            self.sendAuthentificationDenial();
            return self.disconnect();
            return self.serverAuthentification();
        }
        ::std::cerr << "[Connection:TCP:" << self.m_id << "] Authentification successful.\n";
        self.serverSendAuthentificationAcceptance();
    });
}

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverSendAuthentificationAcceptance()
{
    this->sendMessage(
        [](::network::tcp::Connection<UserMessageType>& self){
            ::std::cerr << "[Connection:TCP:" << self.m_id << "] Authentification successful.\n";
            self.m_isValid = true;
            self.m_owner.onConnectionValidated(self.shared_from_this());
        },
        ::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::authentificationAccepted
        }
    );
}



// ------------------------------------------------------------------ async - Client Authentification
// TODO: mem error when closing the client after authentification denial

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientAuthentification()
{
    m_owner.onAuthentification(this->shared_from_this());
    this->clientSendAuthentification();
}

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientSendAuthentification()
{
    this->sendMessage(
        [](::network::tcp::Connection<UserMessageType>& self){
            self.clientReceiveAuthentificationAcceptance();
            ::std::cout << self.getUserName() << ::std::endl;
        },
        ::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::authentification, m_userName, ::std::string("")
        }
    );
}

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientReceiveAuthentificationAcceptance()
{
    this->receiveMessage([](::network::tcp::Connection<UserMessageType>& self){
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
    });
}



// ------------------------------------------------------------------ async - Error Authentification

template <
    typename UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendAuthentificationDenial()
{
    this->sendMessage(
        [](::network::tcp::Connection<UserMessageType>& self){},
        ::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::authentificationDenied
        }
    );
}
