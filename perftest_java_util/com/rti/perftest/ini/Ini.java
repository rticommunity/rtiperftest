/* $Id: Ini.java,v 1.1.2.1 2014/04/01 11:56:55 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
03apr08,rbw Refactored for better error reporting
02apr08,rbw Fixed parsing bugs
02apr08,rbw Created
=========================================================================== */

package com.rti.perftest.ini;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


// ===========================================================================

/**
 * Represents the configuration in a *.ini file. A file is divided into
 * sections, each one which contains some number of name/value pairs.
 */
public final class Ini {
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final Pattern COMMENT_DELIMITER =
        Pattern.compile(";");
    private static final Pattern SECTION_HEADER =
        Pattern.compile("\\[(.+)\\]");
    private static final Pattern KEY_VALUE_DELIMITER =
        Pattern.compile("=");
    
    
    private final Map<String, Section> _namedSections =
        new HashMap<String, Section>();
    
    private final boolean _caseSensitive;



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    public static Ini load(File file, boolean caseSensitive)
    throws IOException {
        return new Ini(file, caseSensitive);
    }


    // -----------------------------------------------------------------------

    public Section section(String name) {
        if (!_caseSensitive) {
            name = name.toLowerCase();
        }
        return _namedSections.get(name);
    }
    
    
    
    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    private Ini(File file, boolean caseSensitive)
    throws IOException {
        assert file != null;
        _caseSensitive = caseSensitive;
        
        BufferedReader reader = new BufferedReader(new FileReader(file));
        try {
            Map<String, String> currentSection = null;
            String line;
            while ((line = reader.readLine()) != null) {
                String[] lineSplitByComments = COMMENT_DELIMITER.split(
                        line, 2);
                assert lineSplitByComments.length == 1 ||   // no comment
                lineSplitByComments.length == 2;     // comment
                String lineBeforeComment = lineSplitByComments[0].trim();
                if (lineBeforeComment.length() == 0) {
                    continue;
                }

                // --- Parse sections --- //
                Matcher matcher = SECTION_HEADER.matcher(lineBeforeComment);
                if (matcher.matches()) {
                    // starting new section:
                    currentSection = new HashMap<String, String>();
                    String sectionName = matcher.group(1);
                    assert sectionName.length() > 0;
                    if (!caseSensitive) {
                        sectionName = sectionName.toLowerCase();
                    }
                    _namedSections.put(
                            sectionName, new Section(currentSection));
                } else {
                    // continuing existing section:
                    assert currentSection != null :
                        "No section declaration in " + file;
                    String[] keyAndValue = KEY_VALUE_DELIMITER.split(
                            lineBeforeComment, 2);
                    assert keyAndValue.length == 2;
                    String key = keyAndValue[0].trim();
                    assert key.length() > 0;
                    String value = keyAndValue[1].trim();
                    assert value.length() > 0;
                    if (!caseSensitive) {
                        key = key.toLowerCase();
                        value = value.toLowerCase();
                    }
                    currentSection.put(key, value);
                }
            }
        } finally {
            reader.close();
        }
    }
    
    
    
    // -----------------------------------------------------------------------
    // Public Classes
    // -----------------------------------------------------------------------

    // =======================================================================
    
    /**
     * Represents one named section of a *.ini file, delimited by a header
     * line like this:
     * 
     * <code>[name]</code>
     */
    public final class Section {
        // --- Private Fields: -----------------------------------------------
        private final Map<String, String> _entries;
        
        // --- Constructors: -------------------------------------------------
        private Section (Map<String, String> entries) {
            _entries = Collections.unmodifiableMap(entries);
        }
        
        // --- Public Methods: -----------------------------------------------
        /**
         * Get the value associated with the given name, or the default if
         * the value doesn't exist.
         */
        public String get(String name, String defaultValue) {
            if (!_caseSensitive) {
                name = name.toLowerCase();
            }
            String value = _entries.get(name);
            if (value == null) {
                value = defaultValue;
            }
            return value;
        }

        /**
         * Get the value associated with the given name as an integer. If
         * the name doesn't exist in this section, or the associated value is
         * not an integer, this method will return the given default value.
         */
        public int getInt(String name, int defaultValue) {
            String strValue = get(name, String.valueOf(defaultValue));
            try {
                return Integer.parseInt(strValue);
            } catch (NumberFormatException nfx) {
                return defaultValue;
            }
        }

        /**
         * Get the value associated with the given name as a boolean. If
         * the name doesn't exist in this section, or the associated value is
         * not a boolean, this method will return the given default value.
         */
        public boolean getBoolean(String name, boolean defaultValue) {
            String strValue = get(name, String.valueOf(defaultValue));
            return Boolean.parseBoolean(strValue);
        }
    }
    
}

// ===========================================================================
// End of $Id: Ini.java,v 1.1.2.1 2014/04/01 11:56:55 juanjo Exp $
