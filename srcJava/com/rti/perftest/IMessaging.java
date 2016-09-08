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

package com.rti.perftest;


// ===========================================================================

/**
 * Runs the performance test.
 */
public interface IMessaging {
    public boolean initialize(int argc, String[] argv);

    public void printCmdLineHelp();

    public void shutdown();

    /**
     * If the implementation supports batching and the test scenario is
     * using batching, this function should return the size of the batch
     * in bytes.
     */
    public int getBatchSize();

    /* Used only for scan mode.
     * The maximum size of a message's binary payload. If the size
     * exceeds this during a scan, the test will stop.
     */
    public int getMaxBinDataSize();

    public IMessagingWriter createWriter(String topicName);
    
    /**
     * Pass null for callback if using IMessagingReader.ReceiveMessage()
     * to get data.
     */
    public IMessagingReader createReader(
            String topicName, IMessagingCB callback);
    
    public void dispose();

}

// ===========================================================================
// End of $Id: IMessaging.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
