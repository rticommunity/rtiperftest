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

import java.util.concurrent.TimeUnit;


// ===========================================================================

/**
 * Send messages.
 */
public interface IMessagingWriter {
    public void waitForReaders(int numReaders);

    public boolean send(TestMessage message);
    public boolean waitForPingResponse();
    public boolean waitForPingResponse(long timeout, TimeUnit unit);
    public boolean notifyPingResponse();

    public void flush();

}

// ===========================================================================
// End of $Id: IMessagingWriter.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
