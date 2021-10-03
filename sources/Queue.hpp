// thread safe queue
// TODO: lock free

#pragma once



namespace network {



template <
    typename Type
> class Queue {

public:

    // ------------------------------------------------------------------ *structors

    inline Queue() = default;

    inline ~Queue()
    {
        this->clear();
    }



    // ------------------------------------------------------------------ front

    [[ nodiscard ]] inline auto front()
        -> Type&
    {
        ::std::scoped_lock lock{ m_mutex };
        return m_deque.front();
    }

    inline void push_front(
        Type item
    )
    {
        ::std::scoped_lock lock{ m_mutex };
        m_deque.emplace_back(::std::move(item));
    }

    [[ nodiscard ]] inline auto pop_front()
        -> Type
    {
        ::std::scoped_lock lock{ m_mutex };
        auto tmpValue{ ::std::move(m_deque.front()) };
        m_deque.pop_front();
        return tmpValue;
    }

    inline void remove_front()
    {
        ::std::scoped_lock lock{ m_mutex };
        m_deque.pop_front();
    }



    // ------------------------------------------------------------------ back

    [[ nodiscard ]] inline auto back()
        -> Type&
    {
        ::std::scoped_lock lock{ m_mutex };
        return m_deque.back();
    }

    inline void push_back(
        Type item
    )
    {
        ::std::scoped_lock lock{ m_mutex };
        m_deque.emplace_back(::std::move(item));
    }

    [[ nodiscard ]] inline auto pop_back()
        -> Type
    {
        ::std::scoped_lock lock{ m_mutex };
        auto tmpValue{ ::std::move(m_deque.back()) };
        m_deque.pop_back();
        return tmpValue;
    }



    // ------------------------------------------------------------------ helpers

    [[ nodiscard ]] inline auto empty()
        -> bool
    {
        ::std::scoped_lock lock{ m_mutex };
        return m_deque.empty();
    }

    [[ nodiscard ]] inline auto count()
        -> bool
    {
        ::std::scoped_lock lock{ m_mutex };
        return m_deque.count();
    }

    inline void clear()
    {
        ::std::scoped_lock lock{ m_mutex };
        m_deque.clear();
    }




private:

    ::std::mutex m_mutex;
    ::std::deque<Type> m_deque;

};



} // namespace network
