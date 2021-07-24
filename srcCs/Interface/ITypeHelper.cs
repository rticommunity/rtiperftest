/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
namespace PerformanceTest
{
    public interface ITypeHelper<T> where T : class, IEquatable<T>
    {
        T MessageToSample(TestMessage message, int keyValue);

        TestMessage SampleToMessage(T dataSequence);

        ITypeHelper<T> Clone();

        /*
         * Calculates the overhead bytes added by all the members on the
         * TestData_* type, excluding the content of the sequence.
         */
        long GetSerializedOverheadSize();
    }
}
