#include <pch.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::detail::Queue<::network::Message<::network::MessageType>>;
template class ::detail::Queue<::network::OwnedMessage<::network::MessageType>>;



// ------------------------------------------------------------------ *structors

template <
    typename Type
> ::detail::Queue<Type>::Queue() = default;

template <
    typename Type
> ::detail::Queue<Type>::~Queue()
{
    this->clear();
}



// ------------------------------------------------------------------ front

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::front()
    -> Type&
{
    ::std::scoped_lock lock{ m_mutexQueue };
    return m_deque.front();
}

template <
    typename Type
> void ::detail::Queue<Type>::push_front(
    Type item
)
{
    {
        ::std::scoped_lock lock{ m_mutexQueue };
        m_deque.emplace_front(::std::move(item));
    }

    this->notify();
}

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::pop_front()
    -> Type
{
    ::std::scoped_lock lock{ m_mutexQueue };
    auto tmpValue{ ::std::move(m_deque.front()) };
    m_deque.pop_front();
    return tmpValue;
}

template <
    typename Type
> void ::detail::Queue<Type>::remove_front()
{
    ::std::scoped_lock lock{ m_mutexQueue };
    m_deque.pop_front();
}



// ------------------------------------------------------------------ back

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::back()
    -> Type&
{
    ::std::scoped_lock lock{ m_mutexQueue };
    return m_deque.back();
}

template <
    typename Type
> void ::detail::Queue<Type>::push_back(
    Type item
)
{
    {
        ::std::scoped_lock lock{ m_mutexQueue };
        m_deque.emplace_back(::std::move(item));
    }

    this->notify();
}

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::pop_back()
    -> Type
{
    ::std::scoped_lock lock{ m_mutexQueue };
    auto tmpValue{ ::std::move(m_deque.back()) };
    m_deque.pop_back();
    return tmpValue;
}

template <
    typename Type
> void ::detail::Queue<Type>::remove_back()
{
    ::std::scoped_lock lock{ m_mutexQueue };
    m_deque.pop_back();
}



// ------------------------------------------------------------------ helpers

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::empty()
    -> bool
{
    ::std::scoped_lock lock{ m_mutexQueue };
    return m_deque.empty();
}

template <
    typename Type
> [[ nodiscard ]] auto ::detail::Queue<Type>::count()
    -> bool
{
    ::std::scoped_lock lock{ m_mutexQueue };
    return m_deque.size();
}

template <
    typename Type
> void ::detail::Queue<Type>::clear()
{
    ::std::scoped_lock lock{ m_mutexQueue };
    m_deque.clear();
}



// ------------------------------------------------------------------ blocking

template <
    typename Type
> void ::detail::Queue<Type>::wait()
{
    ::std::unique_lock lock{ m_mutexBlocker };
    m_blocker.wait(lock);
}

template <
    typename Type
> void ::detail::Queue<Type>::notify()
{
    ::std::scoped_lock lock{ m_mutexBlocker };
    m_blocker.notify_one();
}
