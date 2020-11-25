.. _section-extending_perftest:

===========================================
Extending RTI Perftest to other middlewares
===========================================

The *RTI Perftest* C++ implementation is designed in a way in which it can be
extended to be used to test other middlewares or communication mechanisms.

An example of such is the **RAW transport** support *RTI Perftest* offers when
compiling against *RTI Connext DDS Professional*. In this specific case, although
using specific functions from the OS/Network abstraction layer from *RTI Connext
DDS*, the communication being tested is a pure socket to socket communication
(so, in this case, even simpler than a middleware).

Interface infrastructure
------------------------

The main entry point for the *RTI Perftest* Classic C++ implementation is the
`perftest_cpp` class, under `srcCpp/` There, instead of interacting with directly
with the data types, the *DDS entities* and their methods, the code abstracts
everything and uses the following class interfaces (defined in `srcCpp/MessagingIF`):

- **TestMessage**

This interface contains the different attributed required to interact with the
messages. It can be described as a wrapper of the message that the middleware
sends/receives. Objects of this class will be passed to the `write` methods of
the publication entities.

- **IMessaging**

This interface abstracts the Middleware itself. It contains the methods used to
initialize/finish the middleware, in the case of a *DDS* implementation, the
creation/destruction of *Participants*, *Publishers* and *Subscribers*). It will
also controls the creation of `IMessagingWriter` and `IMessagingReader` entities.

- **IMessagingWriter**

This interface abstracts the entity in the middleware in charge of sending the data,
in the case of a *DDS* implementation, this will be a wrapper for the *DataWriter*.

- **IMessagingReader**

This interface abstracts the entity in the middleware in charge of receiving the data,
in the case of a *DDS* implementation, this will be a wrapper for the *DataReader*.

- **IMessagingCB**

This interface is in charge of processing the messages received by the *IMessagingReader*
interface. That way, independently on how the data was obtained (by pulling data,
by a callback in the middleware or by any other way), the same actions will be done.

Common Infrastructure
---------------------

*RTI Connext DDS Professional* and *Micro* offer specific OS abstraction layers
in order to be able to use semaphores, timers, threads and other OS dependant
functions etc independently on the OS where the Middleware is running.
When *RTI Perftest* is being compiled against those middlewares, it will try to
use that infrastructure. However, when *RTI Perftest* is compiled for anything
else, a default implementation for those functions is provided, based on the
C++11 standard.

The Common infrastructure classes can be found in `srcCpp/Infrastructure_common.h`
and `srcCpp/Infrastructure_common.cxx`.

When adding support for other Middlewares or communication mechanisms, these
functions can be use and extended, or, alternatively, a specific implementation
can be developed to avoid the restriction of using C++11 compilers (this may make
sense if the target OS/Platform does not have a modern compiler).

Message type (idl)
------------------

*RTI Perftest* generates its code for *RTI Connext DDS Professional* and *Micro*
using the *rtiddsgen* tool, which generates the code from some `.idl` files.

These `.idl` files are used to represent the types that *RTI Perftest* uses to
send the messages between publisher and subscriber sides. The `Message` class
mentioned above acts as a wrapper for these types in the .idl.

The files can be found under `srcIdl`. All of them represent the same structure,
but with differences based on attributes that imply the idl representation to
change, e.g.: In order to test with *keyed* data and *unkeyed*, 2 different
structures are needed (one with a member marked as `@key` and the other one without
it).

These files can be used by other middlewares, if these ones accept idl files
as input.

Discovery mechanisms
--------------------

*RTI Perftest* implementation for *RTI Connext DDS* relies on the discovery mechanisms
provided by the middleware. However, this is not a hard restriction, and communication
patterns without discovery can be used as well.

A good example of this is the **RAW transport** implementation *RTI Perftest*
provides.


Command-line options
--------------------

Command line options are parsed in `srcCppCommon/Parameter.h/cxx` and
`srcCppCommon/ParameterManager.h/cxx`, there the infrastructure allow the use
of different masks to determine if a parameter may or may not be parsed and be
available for a certain middleare. This is controlled by the `Middleware::<MiddlewareMask`.

Therefore, enabling existing paramters for different Middleware implementations or
creating new ones should just imply adding the right mask to the parameter.

Build infrastructure
--------------------

*RTI Perftest* relies in two scripts for building, `build.sh` and `build.bat`. These
scripts builds against *RTI Connext DDS* each of the different language implementations.

In every case, *RTI Perftest* for *RTI Connext DDS* uses *rtiddsgen* to generate the
right makefiles/solutions, and then use those files to build natively.

This mechanism may not work for other Middlewares, so *RTI Perftest* cannot offer
for the time being an implementation for building other middlewares, that has to be
implemented by the developer.

Selecting and protecting code
-----------------------------

*RTI Perftest* for *RTI Connext DDS Professional* and *Micro* need to protect
certain parts of the code to avoid interferences, or to let the compiler know
what files it should include or not. This is done by checking if certain defines
are present and provided at compilation time.

Currently *RTI Perftest* makes use of `PERFTEST_RTI_PRO` and `PERFTEST_RTI_MICRO`.
