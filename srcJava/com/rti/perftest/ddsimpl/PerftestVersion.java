/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import com.rti.dds.infrastructure.ProductVersion_t;

public final class PerftestVersion {

    private static final PerftestVersion _singleton = new PerftestVersion();

    private final ProductVersion_t _productVersion;

    private PerftestVersion() {
        _productVersion = new ProductVersion_t(new int[] { 9, 9, 9, 9 });
    }

    public static PerftestVersion getInstance() {
        return _singleton;
    }

    public ProductVersion_t getProductVersion() {
        return _productVersion;
    }
}