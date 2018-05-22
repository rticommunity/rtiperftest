/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.util.HashMap;
import java.util.Map;

public class DynamicDataMembersId {
    private static DynamicDataMembersId instance = null;
    private Map<String, Integer> membersId;

    private DynamicDataMembersId(){
        membersId = new HashMap<String, Integer>();
        membersId.put("key", 1);
        membersId.put("entity_id", 2);
        membersId.put("seq_num", 3);
        membersId.put("timestamp_sec", 4);
        membersId.put("timestamp_usec", 5);
        membersId.put("latency_ping", 6);
        membersId.put("bin_data", 7);
    }
    public static DynamicDataMembersId getInstance() {
        if(instance == null) {
           instance = new DynamicDataMembersId();
        }
        return instance;
    }

    public int at(String key) {
        return (int)membersId.get(key);
    }
}