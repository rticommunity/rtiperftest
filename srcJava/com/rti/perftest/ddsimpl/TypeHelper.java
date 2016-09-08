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

import com.rti.dds.infrastructure.ByteSeq;
import com.rti.dds.topic.TypeSupportImpl;
import com.rti.dds.util.AbstractSequence;
import com.rti.perftest.TestMessage;

public interface TypeHelper<T> {

    public void fillKey(int value);

    public void copyFromMessage(TestMessage message);

    public TestMessage copyFromSeqToMessage(AbstractSequence dataSeq, int i);

    public T getData();

    public ByteSeq getBindata();
    
    public TypeSupportImpl getTypeSupport();

    public TypeHelper<T> clone();

    public AbstractSequence createSequence();

}