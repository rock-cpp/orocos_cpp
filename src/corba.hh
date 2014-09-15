#ifndef OROCOS_RB_EXT_CORBA_HH
#define OROCOS_RB_EXT_CORBA_HH

#include <omniORB4/CORBA.h>

#include <exception>
#include "StdExceptionC.h"
#include "TaskContextC.h"
#include "DataFlowC.h"
#include <iostream>
#include <string>
#include <stack>
#include <list>
#include <stdexcept>
#include <boost/function.hpp>
#include <vector>

namespace RTT
{
    class TaskContext;
    namespace base {
        class PortInterface;
    }
    namespace corba
    {
        class TaskContextServer;
    }
}

struct RTaskContext
{
    RTT::corba::CTaskContext_var         task;
    RTT::corba::CService_var     main_service;
    RTT::corba::CDataFlowInterface_var   ports;
    std::string name;
};

/**
 * This class locates and connects to a Corba TaskContext.
 * It can do that through an IOR.
 */
class CorbaAccess
{
private:
    RTT::TaskContext* m_task;
    RTT::corba::TaskContextServer* m_task_server;
    RTT::corba::CTaskContext_ptr m_corba_task;
    RTT::corba::CDataFlowInterface_ptr m_corba_dataflow;

    CorbaAccess(int argc, char* argv[] );
    ~CorbaAccess();

    RTT::corba::CTaskContext_var getCTaskContext(std::string const& ior);

    static CorbaAccess* the_instance;

public:
    static void init(int argc, char* argv[]);
    static void deinit();
    static CorbaAccess* instance() { return the_instance; }

    void setCallTimeout(int ms);
    void setConnectTimeout(int ms);
    
    std::vector<std::string> getTransportableTypeNames();
    
    /** Returns a new RTaskContext for the given IOR or throws an exception
     *  if the remote task context cannot be reached.
     */
    RTaskContext* createRTaskContext(std::string const& ior);
};


class InvalidIORError :public std::runtime_error
{
    public:
        InvalidIORError(const std::string& what_arg):
            std::runtime_error(what_arg) { }
};

// template<typename F, typename A=boost::function<void()> >
// class CORBABlockingFunction : public BlockingFunction<F,A>
// {
//     public:
//         static void call(F processing, A abort = boost::bind(&BlockingFunctionBase::abort_default))
//         {
//             return BlockingFunctionBase::doCall< void, CORBABlockingFunction<F,A> >(processing, abort);
//         }
// 
//         CORBABlockingFunction(F processing, A abort):
//             BlockingFunction<F, A>(processing, abort) { }
// 
//         virtual void processing()
//         {
//             // add corba exception handlers
//             try { this->processing_fct(); }
//             CORBA_EXCEPTION_HANDLERS
//             EXCEPTION_HANDLERS
//         }
// };
// 
// template<typename F, typename A=boost::function<void()> >
// class CORBABlockingFunctionWithResult : public BlockingFunctionWithResult<F,A>
// {
//     public:
//         typedef typename F::result_type result_t;
//         static result_t call(F processing, A abort = boost::bind(&BlockingFunctionBase::abort_default))
//         {
//             return BlockingFunctionBase::doCall< result_t, CORBABlockingFunctionWithResult<F,A> >(processing, abort);
//         }
// 
//         CORBABlockingFunctionWithResult(F processing, A abort):
//             BlockingFunctionWithResult<F, A>::BlockingFunctionWithResult(processing, abort) { }
// 
//         virtual void processing()
//         {
//             // add corba exception handlers
//             try { this->return_val = this->processing_fct(); }
//             CORBA_EXCEPTION_HANDLERS
//             EXCEPTION_HANDLERS
//         }
// };
// 
// // template functions can automatically pick up their template paramters
// template<typename F, typename A>
// void corba_blocking_fct_call(F processing, A abort)
// {
//     CORBABlockingFunction<F,A>::call(processing,abort);
// }
// 
// template<typename F>
// void corba_blocking_fct_call(F processing)
// {
//     CORBABlockingFunction<F>::call(processing);
// }
// 
// template<typename F, typename A>
// typename F::result_type corba_blocking_fct_call_with_result(F processing, A abort)
// {
//     return CORBABlockingFunctionWithResult<F,A>::call(processing,abort);
// }
// 
// template<typename F>
// typename F::result_type corba_blocking_fct_call_with_result(F processing)
// {
//     return CORBABlockingFunctionWithResult<F>::call(processing);
// }

#endif
