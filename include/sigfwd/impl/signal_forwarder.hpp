// Copyright 2010 Edd Dawson.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef SIGNAL_FORWARDER_HPP_2300_20012011
#define SIGNAL_FORWARDER_HPP_2300_20012011

#include "sigfwd/connect.hpp"

#include <boost/mpl/at.hpp>
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/ref.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <QtCore/QObject>

#include <cassert>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4180) // "qualifier applied to function type has no meaning; ignored"
#endif

namespace sigfwd
{
    namespace impl
    {
        // Reimplements QObject's qt_metacall virtual function. qt_metacall is what gets called to invoke a slot when 
        // a connected signal is emitted.
        class signal_forwarder : public QObject, public boost::noncopyable
        {
            public:
                signal_forwarder(QObject *parent)
                {
                }

                virtual ~signal_forwarder()
                {
                }

                connection connect(QObject *emitter, const char *qt_signal_sig, const char *recv_sig, 
                                   Qt::ConnectionType conn_type, bool check_sigs)
                {
                    assert(emitter);
                    assert(qt_signal_sig);

                    // Qt's SIGNAL(xyz) creates a string that starts with a '2'. '1' would indicate a SLOT.
                    assert(qt_signal_sig[0] == '2');
                    qt_signal_sig++;
                    
                    const QByteArray norm_qt_sig = QMetaObject::normalizedSignature(qt_signal_sig);
                    const QByteArray norm_recv_sig = QMetaObject::normalizedSignature(recv_sig);

                    if (check_sigs && !QMetaObject::checkConnectArgs(norm_qt_sig, norm_recv_sig))
                        return connection(this, sigs_incompatible);

                    const int qt_signal_id = emitter->metaObject()->indexOfSignal(norm_qt_sig);
                    if (qt_signal_id == -1)
                        return connection(this, signal_not_found);
                    
                    // signals and slots in less-derived classes occupy id's that are lower than those
                    // in more-derived classes. 
                    // The first id in this class is therefore equal to the number of signal and slot 
                    // methods in QObject (our base class).
                    const int recv_slot_id = QObject::metaObject()->methodCount();
                    QMetaObject::connect(emitter, qt_signal_id, this, recv_slot_id, conn_type);

                    return connection(this, connected);
                }

                // An override of the method in the QObject base class. 
                // Determines whether the call is for us (or some other sub-object in the inheritance hierarchy) and if
                // so calls forward().
                virtual int qt_metacall(QMetaObject::Call call, int id, void **ret_and_argv)
                {
                    // Can the "meta call" be handled by the base class?
                    int method_id = QObject::qt_metacall(call, id, ret_and_argv);
                    if (method_id == -1 || call != QMetaObject::InvokeMetaMethod)
                        return method_id; // this wasn't meant for our derived meta-caller.

                    // method_id is equal to id less the number of meta-methods in our base-classes.
                    // This gives us an index in to the meta-methods associated with our derived part.
                    // We've only got a single meta-method, so this index should be 0.
                    assert(method_id == 0);

                    // The first pointer of ret_and_argv points to a location where the return value of the
                    // meta-method would be stored. We're going to swallow the return value, so we only need
                    // to pass on pointers to the arguments.
                    void **argv = ret_and_argv + 1;

                    try { forward(argv); }
                    catch (...) { /**GULP**/ }

                    return -1;
                }

            private:
                // To be implemented in a derived class where the arguments of argv are casted appropriately before
                // being passed to a function or functor of some kind.
                virtual void forward(void **argv) = 0;
        };

        template<typename T>
        void maybe_unused(const T&) { }

        template<typename Params, std::size_t N>
        struct nth_param : boost::remove_reference<typename boost::mpl::at_c<Params, N>::type> { };

        template<typename ParamTypes, std::size_t Arity>
        struct call;

#define CAST_NTH_ARGV_TO_ARG_TYPE(z, n, argv) \
    *static_cast<typename nth_param<ParamTypes, n>::type *>(argv[n])
  
#define BOOST_PP_LOCAL_MACRO(n) \
        template<typename ParamTypes> \
        struct call<ParamTypes, n> \
        { \
            template<typename Functor> \
            static void do_it(const Functor &f, void **argv) \
            { \
                maybe_unused(argv); \
                f(BOOST_PP_ENUM(n, CAST_NTH_ARGV_TO_ARG_TYPE, argv)); \
            } \
        };
  
// Make a number of partial specializations of call for different numbers of functor arities.
// A typical macro expansion will look like this:
//
//      template<typename ParamTypes>
//      struct call<ParamTypes, 3>
//      {
//          template<typename Functor>
//          static void do_it(const Functor &f, void **argv)
//          {
//              maybe_unused(argv);
//              f(
//                  *static_cast<typename nth_param<ParamTypes, 0>::type *>(argv[0]),
//                  *static_cast<typename nth_param<ParamTypes, 1>::type *>(argv[1]),
//                  *static_cast<typename nth_param<ParamTypes, 2>::type *>(argv[2])
//              );
//          }
//      };

#define BOOST_PP_LOCAL_LIMITS (0, 10)
#include BOOST_PP_LOCAL_ITERATE()

        // Implements the signal_forwarder interface.
        // Stores a copy of Functor which is invoked in the implementation of forward().
        template<typename Functor, typename FunctorTraits>
        class signal_forwarder_impl : public signal_forwarder
        {
            public:
                signal_forwarder_impl(QObject *parent, const Functor &receiver) :
                    signal_forwarder(parent),
                    receiver_(receiver)
                {
                }

            private:
                virtual void forward(void **argv)
                {
                    typedef typename boost::unwrap_reference<Functor>::type unwrapped_functor;

                    call
                    <
                        typename FunctorTraits::params,
                        FunctorTraits::arity
                    >
                    ::do_it(static_cast<unwrapped_functor &>(receiver_), argv);
                }

            private:
                Functor receiver_;
        };

    } // close namespace impl

} // close namespace sigfwd

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // SIGNAL_FORWARDER_HPP_2300_20012011
