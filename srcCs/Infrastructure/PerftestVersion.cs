/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;

namespace PerformanceTest
{
    public struct PerftestVersion
    {
        public Version version;

        public PerftestVersion(
                int major,
                int minor,
                int release,
                int revision)
        {
            version = new Version(major, minor, release, revision);
        }
    }
} // PerformanceTest namespace
