#ifndef _RTEThreadMan_
#define _RTEThreadMan_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ThreadMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ThreadMan class.
// Project:         Retro Terrain Engine
// Author(s):       
//                  
//                  


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Singleton.h"
#define g_ThreadMan ThreadMan::Instance()

#include "BS_thread_pool.hpp"

namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ThreadMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The centralized singleton manager of all threads.
// Parent(s):       Singleton
// Class history:   03/29/2014  ThreadMan created.


class ThreadMan:
    public Singleton<ThreadMan>
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ThreadMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ThreadMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ThreadMan() { Clear(); Create(); }

    /// <summary>
    /// Makes the TimerMan object ready for use.
    /// </summary>
    void Initialize() { };


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ThreadMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ThreadMan object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ThreadMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ThreadMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ThreadMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ThreadMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


    BS::thread_pool& GetPriorityThreadPool() { return m_PriorityThreadPool; }

    BS::thread_pool& GetBackgroundThreadPool() { return m_BackgroundThreadPool; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:



//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ThreadMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
    ThreadMan(const ThreadMan &reference);
    ThreadMan & operator=(const ThreadMan &rhs);

    // For tasks that we want to be performed ASAP, i.e needs to be complete this frame at some point
    BS::thread_pool m_PriorityThreadPool;

    // For background tasks that we can just let happen whenever over multiple frames
    BS::thread_pool m_BackgroundThreadPool;
};

} // namespace RTE

#endif // File
