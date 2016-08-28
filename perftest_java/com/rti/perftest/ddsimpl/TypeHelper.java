/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
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