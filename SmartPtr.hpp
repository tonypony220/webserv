#include <iostream>
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
	}
	sptr(const sptr & obj) { *this = obj; }

	sptr& operator=(const sptr & obj) 
	{  
		ptr = obj.ptr;
		m_counter = obj.m_counter;
        (*m_counter)++;
		return *this;
	} 

	T* operator->() // deferencing arrow operator
	{
		return this->ptr;
	}

	T& operator*() {
		return *(this->ptr);
	}

	~sptr() // destructor
	{
		(*m_counter)--;
        if (m_counter->get() == 0)
        {
            delete m_counter;
            delete ptr;
        }
		//__cleanup__();
	}

private:
	void __cleanup__()
	{
		if (ptr != nullptr)
			delete ptr;
	}
};
