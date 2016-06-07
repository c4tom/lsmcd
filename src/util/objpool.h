/*
 * Copyright 2002 Lite Speed Technologies Inc, All Rights Reserved.
 * LITE SPEED PROPRIETARY/CONFIDENTIAL.
 */


#ifndef OBJPOOL_H
#define OBJPOOL_H



#include <assert.h>
#include "gpointerlist.h"
typedef int (*ObjFun)(void *pObj, void *param);

class GObjPool
{
    int m_chunkSize;
    int m_poolSize;
    GPointerList m_freeList;
    virtual void *newObj() = 0;
    virtual void   releaseObj(void *pObj) = 0;
    GObjPool(const GObjPool &rhs);
    GObjPool &operator=(const GObjPool &rhs);
protected:
    int allocate(int size);
public:

    explicit GObjPool(int chunkSize = 10);
    virtual ~GObjPool()   {}

    void *get()
    {
        if (m_freeList.empty())
        {
            if (allocate(m_chunkSize))
                return NULL;
        }
        void *pObj = m_freeList.back();
        m_freeList.pop_back();
        return pObj;
    }

    void recycle(void *pObj)
    {
        assert(m_freeList.find(pObj) == m_freeList.end());
        if (pObj)
            m_freeList.safe_push_back(pObj);
    }

    int get(void **pObj, int n)
    {
        if ((int)m_freeList.size() < n)
        {
            if (allocate((n < m_chunkSize) ? m_chunkSize : n))
                return 0;
        }
        m_freeList.safe_pop_back(pObj, n);
        return n;
    }

    void recycle(void **pObj, int n)
    {
        assert(!m_freeList.full());
        if (pObj)
            m_freeList.safe_push_back(pObj, n);
    }

    int size() const                {   return m_freeList.size();   }

    void shrinkTo(int sz)
    {
        int curSize = m_freeList.size();
        int i;
        for (i = 0; i < curSize - sz; ++i)
        {
            void *pObj = m_freeList.back();
            m_freeList.pop_back();
            releaseObj(pObj);
            --m_poolSize;
        }
    }

    void applyAll(ObjFun fn, void *param)
    {
        GPointerList::iterator iter;
        for (iter = begin(); iter != end(); ++iter)
            (*fn)(*iter, param);
    }

    GPointerList::iterator begin()  {   return m_freeList.begin();  }
    GPointerList::iterator end()    {   return m_freeList.end();    }
    void clear()                    {   m_freeList.clear();         }
    int getPoolSize() const         {   return m_poolSize;          }
    int getPoolCapacity() const     {   return m_freeList.capacity();   }
};

template <class _Obj >
class ObjPool : public GObjPool
{

    void *newObj()
    {   return new _Obj();  }

    void releaseObj(void *pObj)
    {   delete(_Obj *)pObj; }

public:

    typedef _Obj **iterator;
    explicit ObjPool(int initSize = 10, int chunkSize = 10)
        : GObjPool(chunkSize)
    {
        if (initSize)
            allocate(initSize);
    }

    ~ObjPool()
    {
        release();
    }
    iterator begin()
    {   return (iterator)GObjPool::begin(); }
    iterator end()
    {   return (iterator)GObjPool::end();   }
    _Obj *get()
    {   return (_Obj *)GObjPool::get();     }
    int get(_Obj **pObj, int n)
    {
        return GObjPool::get((void **)pObj, n);
    }

    void release()
    {
        for (iterator iter = begin(); iter != end(); ++iter)
            if (*iter)
                delete((_Obj *)*iter);
        GObjPool::clear();
    }
};




#endif
