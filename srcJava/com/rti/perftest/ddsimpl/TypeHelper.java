/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

package com.rti.perftest.ddsimpl;

import java.util.List;

import com.rti.dds.topic.TypeSupportImpl;
import com.rti.perftest.TestMessage;

public interface TypeHelper<T> {

    public void fillKey(int value);

    public void copyFromMessage(TestMessage message);

    @SuppressWarnings("rawtypes")
    public TestMessage copyFromSeqToMessage(List dataSeq, int i);

    public T getData();

    public void setBinDataMax(int size);

    public void bindataUnloan();

    public TypeSupportImpl getTypeSupport();

    public TypeHelper<T> clone();

    @SuppressWarnings("rawtypes")
    public List createSequence();

}