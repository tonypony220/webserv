#include <iostream>
#define DEBUG 0
class Counter
{
public:
    // Constructor
    Counter()
        : m_counter(0){};
 
 
    // Destructor
    ~Counter() {}
 
    void reset()
    {
      m_counter = 0;
    }
    unsigned int get()
    {
      return m_counter;
    }
    // Overload post/pre increment
    void operator++()
    {
      m_counter++;
    }
    void operator++(int)
    {
      m_counter++;
    }
    // Overload post/pre decrement
    void operator--()
    {
      m_counter--;
    }
    void operator--(int)
    {
      m_counter--;
    }
    // Overloading << operator
    friend std::ostream& operator<<(std::ostream& os,
                               const Counter& counter)
    {
        os << "Counter Value : " << counter.m_counter
           << std::endl;
        return os;
    }
 
private:
    Counter(const Counter&);
    Counter& operator=(const Counter&);
    unsigned int m_counter;
};


template <class T>
class sptr
{
private:
	T * 	 ptr;
    Counter* m_counter;

public:
	//sptr() : ptr(nullptr) {} // default constructor
	explicit sptr(T * ptr) : ptr(ptr) {
		m_counter = new Counter();
        if (ptr)
        {
            (*m_counter)++;
        }
		DEBUG && std::cout << PURPLE"smart ptr ctor: " << *this << "\n"RESET; // ptr= " << this->ptr << " counter" << m_counter->get() << "\n";
	}
	sptr(const sptr & obj) {
		ptr = obj.ptr;
		m_counter = obj.m_counter;
		(*m_counter)++;
//		*this = obj;
		DEBUG && std::cout << PURPLE"smart ptr copy: " << *this << "\n"RESET; // ptr= " << this->ptr << " counter" << m_counter->get() << "\n";
	}

	sptr& operator=(const sptr & obj) 
	{
		if (this != &obj) {
			__cleanup__();
			ptr = obj.ptr;
			m_counter = obj.m_counter;
			(*m_counter)++;
		}
		DEBUG && std::cout << PURPLE"smart ptr assigment: " << *this << "\n"RESET; // ptr= " << this->ptr << " counter" << m_counter->get() << "\n";
		return *this;
	} 

	T* operator->() // deferencing arrow operator
	{
		return this->ptr;
	}

	T* get_ptr() // deferencing arrow operator
	{
		return this->ptr;
	}
	int get_num() // deferencing arrow operator
	{
		return m_counter->get();
	}

	T& operator*()
	{
		return *(this->ptr);
	}


	~sptr() // destructor
	{
		DEBUG && std::cout << PURPLE"smart ptr dtor: " << *this << "\n"RESET;
//		std::cout << "smart ptr destructor: ptr= " << this->ptr << " counter" << m_counter->get() << "\n";
//		std::cout << "smart ptr destructor: " << *this << "\n"; // ptr= " << this->ptr << " counter" << m_counter->get() << "\n";
		__cleanup__();
	}

private:
	void __cleanup__()
	{
		(*m_counter)--;
		if (m_counter->get() == 0)
		{
			delete m_counter;
//			if (ptr != nullptr)
			delete ptr;
//            delete ptr;
		}
	}
};

template <class T>
std::ostream & operator<<( std::ostream & o, sptr<T> & p ) {
	o << p.get_ptr();
	o << " " << p.get_num();
	return ( o );
}
