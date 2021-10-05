// thread safe queue
// TODO: lock free

#pragma once



namespace detail {



template <
    typename Type
> class Queue {

public:

    // ------------------------------------------------------------------ *structors

    Queue();

    ~Queue();



    // ------------------------------------------------------------------ front

    [[ nodiscard ]] auto front()
        -> Type&;

    void push_front(
        Type item
    );

    [[ nodiscard ]] auto pop_front()
        -> Type;

    void remove_front();



    // ------------------------------------------------------------------ back

    [[ nodiscard ]] auto back()
        -> Type&;

    void push_back(
        Type item
    );

    [[ nodiscard ]] auto pop_back()
        -> Type;

    void remove_back();



    // ------------------------------------------------------------------ helpers

    [[ nodiscard ]] auto empty()
        -> bool;

    [[ nodiscard ]] auto count()
        -> bool;

    void clear();



    // ------------------------------------------------------------------ blocking

    void wait();

    void notify();



private:

    ::std::mutex m_mutexQueue;
    ::std::deque<Type> m_deque;

    ::std::mutex m_mutexBlocker;
    ::std::condition_variable m_blocker;

};



} // namespace detail
