// Copyright 2010 Edd Dawson.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef CONNECTION_HPP_0034_08072010
#define CONNECTION_HPP_0034_08072010

#include <QObject>

namespace sigfwd
{
    enum result 
    { 
        connected = 0,           //!< a signal was connected successfully
        sigs_incompatible = -1,  //!< a signal couldn't be connected due to a call signature incompatibility
        signal_not_found = -2,   //!< the signal couldn't be found in the emitter QObject
        disconnected = -3,       //!< the signal was disconnected by the user
    };

    //! A wrapper around a result and a QObject* that effectively is the connection. 
    //! In boolean contexts, evaluates to true iff the internal status member is equal to connected.
    struct connection
    {   
        typedef result connection::* boolean_type;

        //! Conversion constructor from a result
        connection(QObject *qobj, result status) : status(status), qobj(qobj) 
        {
            if (status != connected) {
                free_qobj();
            }
        }
        
        //! Disconnects the signal by deleting qobj.
        void disconnect()
        {
            free_qobj();
            status = disconnected;
        }

        //! Conversion operator to an object that evaluates to true in boolean contexts
        operator boolean_type() const
        {
            return status == connected ? &connection::status : 0; 
        }

        result status; //!< The status of the connection.

      private:
        void free_qobj()
        {
            if (qobj) {
              delete qobj;
              qobj = 0;
            }
        }

        QObject *qobj; //!< delete this to disconnect. May be null if status != connected.
    };

} // close namespace sigfwd

#endif // CONNECTION_HPP_0034_08072010
