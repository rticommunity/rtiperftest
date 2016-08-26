/*

modification history:
--------------------
 5.1.0,11aug14,jm PERFTEST-69 Added -keyed command line option.

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