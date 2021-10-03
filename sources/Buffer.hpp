#pragma once

namespace network {



class Buffer {

public:

    // ------------------------------------------------------------------ *structors

    Buffer();

    ~Buffer();



private:

    ::std::vector<::std::bytes> m_data;

};



} // namespace network
