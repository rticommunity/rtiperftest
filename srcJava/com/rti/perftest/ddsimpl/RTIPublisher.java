/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.rti.perftest.ddsimpl;


import com.rti.dds.infrastructure.InstanceHandle_t;
import com.rti.dds.infrastructure.RETCODE_ERROR;
import com.rti.dds.infrastructure.RETCODE_NO_DATA;
import com.rti.dds.publication.DataWriter;
import com.rti.dds.publication.PublicationMatchedStatus;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;

import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

// ===========================================================================

/*package*/ final class RTIPublisher<T> implements IMessagingWriter {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private DataWriter _writer;
    private TypeHelper<T> _typeHelper = null;

    private int _numInstances;
    private long _instanceCounter = 0;
    InstanceHandle_t[] _instanceHandles;
    private Semaphore _pongSemaphore = new Semaphore(0,true);

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public RTIPublisher(DataWriter writer,int num_instances, TypeHelper<T> myDataType) {
        _typeHelper = myDataType;
        _writer = writer;
        _numInstances = num_instances;
        _instanceHandles = new InstanceHandle_t[num_instances];
        _typeHelper.getBindata().setMaximum(0);

        for (int i = 0; i < _numInstances; ++i) {
            _typeHelper.fillKey(i);
            _instanceHandles[i] = _writer.register_instance_untyped(_typeHelper.getData());
        }
        
    }


    // --- IMessagingWriter: -------------------------------------------------

    public void flush() {
        _writer.flush();
    }


    public boolean send(TestMessage message) {
        int key = 0;

        _typeHelper.copyFromMessage(message);

        if (_numInstances > 1) {
            key = (int) (_instanceCounter++ % _numInstances);
            _typeHelper.fillKey(key);
        }
       
        try {
            _writer.write_untyped(_typeHelper.getData(), _instanceHandles[key]);
        } catch (RETCODE_NO_DATA ignored) {
            // nothing to do
        } catch (RETCODE_ERROR err) {
            System.out.println("Write error: " + err.getMessage());
            return false;
        } finally {
            _typeHelper.getBindata().unloan();
        }

        return true;
    }

    public void waitForReaders(int numSubscribers) {
        PublicationMatchedStatus status = new PublicationMatchedStatus();

        while (true) {
            _writer.get_publication_matched_status(status);
            if (status.current_count >= numSubscribers) {
                break;
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException ix) {
                System.out.println("Wait interrupted");
                return;
            }
        }
    }

    public boolean waitForPingResponse() 
    {
        if(_pongSemaphore != null) 
        {

            try {
                _pongSemaphore.acquire();
            } catch ( InterruptedException ie ) {
                System.out.println ("Acquire interrupted");
                return false;
            }

        }
        return true;
    }

    public boolean waitForPingResponse(long timeout, TimeUnit unit) 
    {
        if(_pongSemaphore != null) 
        {
            try {
                _pongSemaphore.tryAcquire(timeout, unit);
            } catch ( InterruptedException ie ) {
                System.out.println ("Acquire interrupted");
                return false;
            }

        }
        return true;
    }

    public boolean notifyPingResponse() 
    {
        if(_pongSemaphore != null) 
        {
            try {
                _pongSemaphore.release();
            } catch ( Exception e ) {
                System.out.println("Release");
                return false;
            }
        }
        return true;
    }

}

// ===========================================================================
// End of $Id: RTIPublisher.java,v 1.4 2014/08/12 10:21:28 jmorales Exp $
