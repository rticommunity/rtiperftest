#ifndef __PROPERTY_H__
#define __PROPERTY_H__

/* $Id: Property.h,v 1.1.2.1 2014/04/01 11:56:52 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

  Modification history
  --------------------
  1.0a,19mar08,hhw Copied from RTIMessaging.
  1.0a,25jan07,fb  Created.
===================================================================== */

#include <exception>
#include <string>
#include <iostream>
#include <fstream>

#include <stdlib.h>

#ifdef RTI_UNIX
  #include <stdexcept>
#endif

#include "ndds/ndds_cpp.h"


/*e \ingroup Property
 *  \brief Exception thrown during a search by key when the key was 
 *         not found
 */
struct not_found: public std::logic_error {
    explicit not_found(const std::string &msg): std::logic_error(std::string("not_found: ").append(msg)) {}
};

/*e \ingroup Property
 *  \brief Exception thrown during a generic data read operation 
 *         between incompatible types
 */
struct invalid_type: public std::logic_error {
    explicit invalid_type(const std::string &msg): std::logic_error(std::string("invalid_type: ").append(msg)) {}
};

/*e \ingroup Property
 *  \brief the exception thrown when an attempt to change a read-only 
 *         object was made
 */
struct read_only: public std::logic_error {
    explicit read_only(const std::string &desc) throw(): logic_error(std::string("read_only: ").append(desc)) {}
};

/*e \ingroup Property
 *  \brief the exception thrown for section not completed yet
 */
struct todo: public std::logic_error {
    explicit todo(const std::string &desc) throw(): logic_error(std::string("todo: ").append(desc)) {}
};

/*e \ingroup Property
 *  \brief Format error are generic errors reported when some formatting or
 *         parsing is performed, and some illegal characters or data is found.
 */
struct format_error: public std::runtime_error {
    explicit format_error(const std::string &desc) throw(): runtime_error(std::string("format_error ").append(desc)) {}
};

/* forward declarations */
class ValueMapImpl;
class QosDictionaryImpl;

/*e
  \ingroup Property
  
  @brief
  A simple set of utility classes to load .ini files
*/

// ----------------------------------------------------------------------------
/*e \ingroup Property
 * @brief A simple key-value map used for properties.
 *
 * Operator = and copy constructor creates an exact copy of the map, including
 * the 'read_only' flag. To create a new map that is writable, use the 'clone'
 * and 'clone_to' methods.
 */
class ValueMap 
{
private:
    ValueMapImpl *  _theImpl;

protected:
    bool _isReadOnly;

public:

    /*e @brief Converts a string into an int 
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_int(const std::string &valueIn, int *valueOut = NULL);
    /*e @brief Converts a string into an unsigned int 
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_uint(const std::string &valueIn, unsigned int *valueOut = NULL);
    /*e @brief Converts a string into an unsigned int greater than 0
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_uint_gt0(const std::string &valueIn, unsigned int *valueOut = NULL);
    /*e @brief Converts a string into an unsigned int greater than or equal to 0
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_uint_gte0(const std::string &valueIn, unsigned int *valueOut = NULL);
    /*e @brief Converts a string into a bool
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_bool(const std::string &valueIn, bool *valueOut = NULL);
    /*e @brief Converts a string into a double
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_double(const std::string &valueIn, double *valueOut = NULL);
    /*e @brief Converts a string into a DDS_Duration_t
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_duration(const std::string &valueIn, DDS_Duration_t *valueOut = NULL);
    /*e @brief Converts a string into a string
      The output value is optional and is not set if NULL
      @throws invalid_type if the string cannot be converted to the given type
     */
    static void parse_as_string(const std::string &valueIn, std::string *valueOut = NULL) {
        if (valueOut) *valueOut = valueIn;
    }
    
public:
    /*e @brief Constructor */
    ValueMap();
    /*e @brief Copy constructor */
    ValueMap(const ValueMap &ref);
    /*e @brief Destructor */
    virtual ~ValueMap();
    
    ValueMap & operator=(const ValueMap &ref);

    /*e @brief Creates a duplicate */
    ValueMap clone() const;    

    /*e @brief Duplicates itself into reference */
    void clone_to(ValueMap &map) const;

    /*e @brief Merges its contents into reference */
    void merge_to(ValueMap &map) const;

    /*e @brief Returns if is set to be read-only */
    bool is_read_only() const {
        return _isReadOnly;
    }

    /*e @brief Sets this map as read only.
     *  Note: once a map has been set to read-only you cannot make it writable
     *        again.
     */
    void set_read_only() {
        _isReadOnly = true;
    }
    
    /*e @brief Returns if the key exists in the map */
    bool is_set(const std::string &key) const;

    /*e @brief Undefine a particular key.
      @throws read_only - if map is in read-only state */
    bool unset(const std::string &key);

    /*e @brief sets a bool value for the key (destroys any previous value)
      @throws read_only - if map is in read-only state */
    void set_bool(const std::string &key, bool value);

    /*e @brief sets an int value for the key (destroys any previous value)
      @throws read_only - if map is in read-only state */
    void set_int(const std::string &key, int value);

    /*e @brief sets a string value for the key (destroys any previous value)
      @throws read_only - if map is in read-only state */
    void set_string(const std::string &key, const std::string &value) ;

    /*e @brief sets a doubl value for the key (destroys any previous value)
      @throws read_only - if map is in read-only state */
    void set_double(const std::string &key, double value);

    /*e @brief sets a DDS_Duration_t value for the key 
      (destroys any previous value)
      @throws read_only - if map is in read-only state */
    void set_duration(const std::string &key, const DDS_Duration_t &value);

    /*e @brief returns value for the key as a bool
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not a bool */
    bool get_bool(const std::string &key) const;

    /*e @brief returns value for the key as an unsigned int
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not an unsigned int */
    unsigned int get_uint(const  std::string &key) const;

    /*e @brief returns value for the key as an int
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not an int */
    int get_int(const  std::string &key) const;

    /*e @brief returns value for the key as a string
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not a string */
    const std::string &get_string(const std::string &key) const;

    /*e @brief returns value for the key as a double
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not a double */
    double get_double(const std::string &key) const;

    /*e @brief returns value for the key as a DDS_Duration_t
      @throws not_found - if key is not defined
      @throws invalid_type - if corresponding value is not a DDS_Duration_t*/
    void get_duration(const std::string &key, DDS_Duration_t &durationOut) const;

    /*e @brief returns value for the key as a bool
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not a bool */
    bool get_bool(const std::string &key, bool default_value) const {
        return (is_set(key) ? get_bool(key) : default_value);
    }

    /*e @brief returns value for the key as an unsigned int
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not an unsigned int */
    unsigned int get_uint(const std::string &key, unsigned int default_value) const {
        return (is_set(key) ? get_uint(key) : default_value);
    }

    /*e @brief returns value for the key as an int 
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not an int */
    int get_int(const std::string &key, int default_value) const {
        return (is_set(key) ? get_int(key) : default_value);
    }

    /*e @brief returns value for the key as a string
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not a string */
    const std::string &get_string(const std::string &key, const std::string &default_value) const {
        return (is_set(key) ? get_string(key) : default_value);
    }

    /*e @brief returns value for the key as a string
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not a string */
    const char *get_string(const std::string &key, const char *default_value) const {
        return (is_set(key) ? get_string(key).c_str() : default_value);
    }

    /*e @brief returns value for the key as a double
      If not found will return the default_value passed in.
      @throws invalid_type - if corresponding value is not a double */
    double get_double(const std::string &key, RTI_DOUBLE64 default_value) const {
        return (is_set(key) ? get_double(key) : default_value);
    }


    /*e @brief Returns the number of keys stored in this map */
    unsigned int size() const;

    /*e @brief  Empties the contents of the map */
    void clear();
    
public:
    // Set Validator
    class Validator {
    public:
        virtual ~Validator() {}
        virtual bool validate(const std::string &key, const std::string &val) const = 0;
    };
    
    bool validate_set(const Validator &validator) const;

public:
    void serialize_to(std::ostream &os) const;
    void deserialize_from(std::istream &is);
};




// ----------------------------------------------------------------------------
/*e \ingroup Property
 *  @brief A QosProfile is simply ValueMap with a name.
 *
 */
class QosProfile: public ValueMap 
{
public:
    /*e typedef for profile name */
    typedef std::string QosProfileName;
    
    /*e ANONYMOUS_NAME */
    static const QosProfileName ANONYMOUS_NAME;
    /*e EMPTY_PROFILE */
    static const QosProfile     EMPTY_PROFILE;
    
protected:
    QosProfileName _theName;
    
public:
    /*e @brief Will create a QosProfile with ANONYMOUS_NAME */
    QosProfile():
        ValueMap(),
        _theName(ANONYMOUS_NAME) {
    }

    /*e @brief Constructor */
    QosProfile(const QosProfileName &name):
        ValueMap(),
        _theName(name){ 
    }
    
    /*e @brief Copy constructor */
    QosProfile(const QosProfile &ref):
        ValueMap(ref),
        _theName(ref._theName) {
    }

    /*e @brief Destructor */
    virtual ~QosProfile() { }
    
    QosProfile & operator=(const QosProfile &ref) {
        ValueMap::operator=(ref);
        _theName = ref._theName;
        return *this;
    }

    /*e @brief Clone */
    QosProfile clone() const {
        QosProfile retVal(_theName);
        ValueMap::clone_to(retVal);
        return retVal;
    }
    
    /*e @brief Clone */
    void clone_to(QosProfile &map) const {
        ValueMap::clone_to(map);
        map._theName = _theName;
    }


    /*e @brief Merge */
    void merge_to(QosProfile &map) const {
        ValueMap::merge_to(map);
        map._theName = _theName;
    }

    /*e @brief Sets the name
        @throws read_only - if the ValueMap is read-only */
    void set_name(const QosProfileName &name) {
        if (_isReadOnly) {
            throw read_only(name);
        }
        _theName = name;
    }
    
    /*e @brief Gets the name */
    QosProfileName get_name() const {
        return _theName;
    }
};



// ----------------------------------------------------------------------------
/*e \ingroup Property
 * @brief A QosDictionary is a collection of QosProfile objects
 *
 * Profiles are organized inside a qos dictionary by name. You can add, search
 * or delete QosProfile objects.
 *
 * All the objects are passed by value.
 *
 * A QosDictionary implements value copy by a destructive copy (like auto_ptr).
 * Use the clone() method to make a physical copy.
 */
class QosDictionary {
private:
    QosDictionaryImpl * _theImpl;

    /*e @brief Copy construction is not allowed
     */
    QosDictionary(const QosDictionary &/*ref*/) { }
    

    /*e @brief Assignments is not allowed, you may want to consider clone()
     */
    QosDictionary &operator=(const QosDictionary &ref);
    

public:
    /*e @brief Builds an empty qos dictionary.
     */
    QosDictionary();
    
    /*e @brief Builds a qos dictionary with only one QosProfile object
     */
    QosDictionary(const QosProfile &profile);
    
    /*e @brief Builds a qos dictionary from the given file.
     *
     *  The file format supported is Windows .INI style, 
     *  and read the entire content in memory.
     */
    QosDictionary(const std::string &filename);

    /*e @brief  Builds a Qos dictionary from the given input stream
     * The stream is parsed and interpreted as an INI file with like 
     * the following example:
     *
     * <pre>
     *      # This is a comment
     *      ; This is also a comment
     *      [ MyProfileName ]
     *          key = value
     *          key = value
     *      [ MyName ]
     *          key = value
     * </pre>
     *
     * Whitespaces are ignored.
     */
    QosDictionary(std::istream &source);

    
    /*e @brief Sestructor
     */
    virtual ~QosDictionary();
    
    /*e @brief Clones the current dictionary */
    QosDictionary clone() const;
    
    /*e @brief Add or replace a profile
     */
    void add(const QosProfile &profile);
    
    /*e @brief Gets an existing profile. 
     * @return NULL if name is not defined
     */
    QosProfile *get(const QosProfile::QosProfileName &name) const;
    
    /*e @brief Remove an existing.
     * @return true if removed, false if not found.
     */
    bool remove(const QosProfile::QosProfileName &name);
                
    /*e @brief Returns the number of profiles stored in this dictionary
     */
    unsigned int size() const;
};


#endif //  __PROPERTY_H__
