
/**
 * A helper class to simulate a set of unsigned integers.
 * Very much like boost::dynamic_bitset, but with focus on set operation.
 */

#pragma once

#include <cstring>
#include <initializer_list>
#include <memory>

class uint_set_iterator
{
public:
    uint_set_iterator(unsigned char *bytes, unsigned bytes_size, unsigned i, unsigned j)
        : m_bytes(bytes), m_bytes_size(bytes_size), m_i(i), m_j(j)
    {
    }

    unsigned int operator*()
    {
        return m_i * 8 + m_j;
    }

    uint_set_iterator &operator++()
    {
        while (m_i < m_bytes_size)
        {
            if (++m_j >= 8)
            {
                m_j = 0;
                if (++m_i >= m_bytes_size)
                {
                    break;
                }
            }
            if (m_bytes[m_i] & (1 << m_j))
            {
                break;
            }
        }
        return *this;
    }

    friend bool operator==(const uint_set_iterator &lhs, const uint_set_iterator &rhs)
    {
        return lhs.m_i == rhs.m_i && lhs.m_j == rhs.m_j;
    }

    friend bool operator!=(const uint_set_iterator &lhs, const uint_set_iterator &rhs)
    {
        return !(lhs == rhs);
    }

private:
    unsigned char *m_bytes;
    unsigned m_bytes_size;
    unsigned m_i;
    unsigned m_j;
};

class uint_set
{
public:
    uint_set()
    {
        allocate(128); // sufficient for integers in range [0, 1024)
    }

    uint_set(const std::initializer_list<unsigned int> &ints)
    {
        allocate(128);
        for (const auto &i : ints)
        {
            add(i);
        }
    }

    ~uint_set()
    {
        deallocate();
    }

    uint_set(const uint_set &other)
    {
        allocate(other.m_bytes_size);
        memcpy(m_bytes, other.m_bytes, m_bytes_size);
    }

    uint_set(uint_set &&other)
    {
        swap(other);
    }

    uint_set &operator=(const uint_set &other)
    {
        uint_set temp(other);
        swap(temp);
        return *this;
    }

    uint_set &operator=(uint_set &&temp)
    {
        swap(temp);
        return *this;
    }

    /**
     * Read-only iterators
     */
    uint_set_iterator begin() const
    {

        for (unsigned i = 0; i < m_bytes_size; ++i)
        {
            for (unsigned j = 0; j < 8; ++j)
            {
                if (m_bytes[i] & (1 << j))
                {
                    return uint_set_iterator(m_bytes, m_bytes_size, i, j);
                }
            }
        }
        return end();
    }

    uint_set_iterator end() const
    {
        return uint_set_iterator(m_bytes, m_bytes_size, m_bytes_size, 0);
    }

    /**
     * Check if integer i is in the set.
     */
    bool has(unsigned int i) const
    {
        unsigned bytes_i = i / 8;
        unsigned bytes_j = i % 8;
        return bytes_i < m_bytes_size && (m_bytes[bytes_i] & (1 << bytes_j));
    }

    /**
     * Shift each byte by @n to the right.
     * Has the effect of adding 8 * @n to every element.
     */
    void rshift(unsigned int n)
    {
        auto new_size = m_bytes_size + n;
        auto temp = new unsigned char[new_size];
        std::memset(temp, 0, n);
        std::memcpy(temp + n, m_bytes, m_bytes_size);
        deallocate();
        m_bytes = temp;
        m_bytes_size = new_size;
    }

    /**
     * The maximum number of elements the set can currently holds.
     */
    unsigned int capacity()
    {
        return m_bytes_size * 8;
    }

    /**
     * Add integer i to the set.
     */
    void add(unsigned int i)
    {
        unsigned bytes_i = i / 8;
        unsigned bytes_j = i % 8;
        allocate(bytes_i + 1);
        m_bytes[bytes_i] |= (1 << bytes_j);
    }

    /**
     * Remove integer i from the set.
     * If i is not in the set, do nothing.
     */
    void remove(unsigned int i)
    {
        unsigned bytes_i = i / 8;
        unsigned bytes_j = i % 8;
        if (bytes_i < m_bytes_size)
        {
            m_bytes[bytes_i] &= ~(1 << bytes_j);
        }
    }

    /**
     * Check whether the set is empty.
     */
    bool empty() const
    {
        for (unsigned i = 0; i < m_bytes_size; ++i)
        {
            if (m_bytes[i])
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Remove elements (make the set empty).
     */
    void clear()
    {
        std::memset(m_bytes, 0, m_bytes_size);
    }

    /**
     * If the set is non-empty, pop an integer from the set and its value will be passed to @el.
     * Otherwise, do nothing and returns false.
     */
    bool pop(unsigned &el)
    {
        for (unsigned i = 0; i < m_bytes_size; ++i)
        {
            if (m_bytes[i])
            {
                for (unsigned j = 0; j < m_bytes_size; ++j)
                {
                    if (m_bytes[i] & (1 << j))
                    {
                        el = i * 8 + j;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     * Check if intersection with @other is empty.
     */
    bool intersect(const uint_set &other) const
    {
        auto n = std::min(m_bytes_size, other.m_bytes_size);
        for (unsigned i = 0; i < n; ++i)
        {
            if (m_bytes[i] & other.m_bytes[i])
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Equality check
     */
    friend bool operator==(const uint_set &lhs, const uint_set &rhs)
    {
        if (lhs.m_bytes_size >= rhs.m_bytes_size)
        {
            return std::memcmp(lhs.m_bytes, rhs.m_bytes, rhs.m_bytes_size) == 0 && !nonzero(lhs.m_bytes + rhs.m_bytes_size, lhs.m_bytes + lhs.m_bytes_size);
        }
        else
        {
            return std::memcmp(lhs.m_bytes, rhs.m_bytes, lhs.m_bytes_size) == 0 && !nonzero(rhs.m_bytes + lhs.m_bytes_size, rhs.m_bytes + rhs.m_bytes_size);
        }
    }

    friend bool operator!=(const uint_set &lhs, const uint_set &rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * Take union in-place.
     */
    uint_set &operator|=(const uint_set &other)
    {
        auto n = std::max(m_bytes_size, other.m_bytes_size);
        allocate(n);
        for (unsigned i = 0; i < other.m_bytes_size; ++i)
        {
            m_bytes[i] |= other.m_bytes[i];
        }
        return *this;
    }

    /**
     * Take union.
     */
    friend uint_set operator|(const uint_set &lhs, const uint_set &rhs)
    {
        uint_set temp(lhs);
        temp |= rhs;
        return temp;
    }

    /**
     * Take intersection in-place.
     */
    uint_set &operator&=(const uint_set &other)
    {
        auto n = std::min(m_bytes_size, other.m_bytes_size);
        for (unsigned i = 0; i < n; ++i)
        {
            m_bytes[i] &= other.m_bytes[i];
        }
        for (unsigned i = n; i < m_bytes_size; ++i)
        {
            m_bytes[i] = 0;
        }
        return *this;
    }

    /**
     * Take intersection.
     */
    friend uint_set operator&(const uint_set &lhs, const uint_set &rhs)
    {
        uint_set temp(lhs);
        temp &= rhs;
        return temp;
    }

private:
    static bool nonzero(unsigned char *first, unsigned char *last)
    {
        while (first != last)
        {
            if (*first++)
            {
                return true;
            }
        }
        return false;
    }

    void allocate(unsigned int size)
    {
        if (m_bytes_size < size)
        {
            auto temp_bytes = new unsigned char[size];
            if (m_bytes_size > 0)
            {
                std::memcpy(temp_bytes, m_bytes, m_bytes_size);
            }
            std::memset(temp_bytes + m_bytes_size, 0, size - m_bytes_size);
            deallocate();
            m_bytes_size = size;
            m_bytes = temp_bytes;
        }
    }

    void deallocate()
    {
        if (m_bytes_size > 0 && m_bytes)
        {
            delete[] m_bytes;
            m_bytes = nullptr;
            m_bytes_size = 0;
        }
    }

    void swap(uint_set &other)
    {
        std::swap(m_bytes, other.m_bytes);
        std::swap(m_bytes_size, other.m_bytes_size);
    }

    unsigned int m_bytes_size{0};
    unsigned char *m_bytes{nullptr};
};
