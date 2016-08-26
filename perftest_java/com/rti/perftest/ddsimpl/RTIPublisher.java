/* $Id: RTIPublisher.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
09jul10,jsr Added new waitForPingResponse with timeout
07jul10,eys Fixed return value for wiatForPingResponse and notifyPingResponse
03may10,jsr Added waitForPingResponse and notifyPingResponse
21may09,fcs Optimized send for unkeyed topics
14aug08,ch  optimized changing the key value before write
09aug08,ch  Key support, multi-instances, durability
01may08,hhw Removed singleCore option.
18apr08,eys Added loan() and unloan() feature to octet sequence
03apr08,rbw Fixed byte copy
02apr08,rbw Fixed syntax error in printf() calls
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

package com.rti.perftest.ddsimpl;

import com.rti.dds.infrastructure.InstanceHandle_t;
import com.rti.dds.infrastructure.RETCODE_ERROR;
import com.rti.dds.infrastructure.RETCODE_NO_DATA;
import com.rti.dds.publication.DataWriter;
import com.rti.dds.publication.PublicationMatchedStatus;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestData_t;
import com.rti.perftest.gen.TestData_tDataWriter;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;


// ===========================================================================

/*package*/ final class RTIPublisher implements IMessagingWriter {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private TestData_tDataWriter _writer;
    private TestData_t _data = new TestData_t();


    private int _numInstances;
    private long _instanceCounter = 0;
    InstanceHandle_t[] _instanceHandles;
    private Semaphore _pongSemaphore = new Semaphore(0,true);

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    public RTIPublisher(DataWriter writer,int num_instances) {
        _writer = (TestData_tDataWriter)writer;
        _data.bin_data.setMaximum(0);
        _numInstances = num_instances;
        _instanceHandles = new InstanceHandle_t[num_instances];

        for (int i = 0; i < _numInstances; ++i) {
           _data.key[0] = (byte) (i);
           _data.key[1] = (byte) (i >>> 8);
           _data.key[2] = (byte) (i >>> 16);
           _data.key[3] = (byte) (i >>> 24);
          
           _instanceHandles[i] = _writer.register_instance(_data);
        }
    }


    // --- IMessagingWriter: -------------------------------------------------

    public void flush() {
        _writer.flush();
    }


    public boolean send(TestMessage message) {
        int key = 0;

        _data.entity_id = message.entity_id;
        _data.seq_num = message.seq_num;
        _data.timestamp_sec = message.timestamp_sec;
        _data.timestamp_usec = message.timestamp_usec;
        _data.latency_ping = message.latency_ping;
        _data.bin_data.loan(message.data, message.size);

        if (_numInstances > 1) {
            key = (int) (_instanceCounter++ % _numInstances);

            _data.key[0] = (byte) (key);
            _data.key[1] = (byte) (key >>> 8);
            _data.key[2] = (byte) (key >>> 16);
            _data.key[3] = (byte) (key >>> 24);
        }
       
        try {
            _writer.write(_data, _instanceHandles[key]);
        } catch (RETCODE_NO_DATA ignored) {
            // nothing to do
        } catch (RETCODE_ERROR err) {
            System.out.println("Write error: " + err.getMessage());
            return false;
        } finally {
            _data.bin_data.unloan();
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
// End of $Id: RTIPublisher.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
