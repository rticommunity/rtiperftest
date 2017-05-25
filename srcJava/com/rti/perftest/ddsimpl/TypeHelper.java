/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
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

    @SuppressWarnings("rawtypes")
    public int getMaxPerftestSampleSize();

}