/* $Id: Property.cxx,v 1.1.2.1 2014/04/01 11:56:51 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

  Modification history
  --------------------
  1.0a,19mar08,hhw Copied from RTIMessaging.
  1.0a,25jan07,fb  Created.
===================================================================== */

#include "Property.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>

#ifdef RTI_WIN32
  #include <hash_map>
  #include <hash_set>
#else
  #include <ext/hash_map>
  #include <ext/hash_set>

// Template specialization for hash<T>() to handle std::string objects
namespace __gnu_cxx {
    template<> struct hash<std::string> {
        size_t operator()(const std::string &__x) const {
            return hash<const char *>()(__x.c_str());
        }
    }; // hash_compare
} // namespace stdext

#endif

#define MAX_LINE    (512)

#ifdef RTI_WIN32
#pragma warning(push)
#pragma warning (disable:4996)
#endif

#ifdef RTI_WIN32
#define RTI_SNPRINTF        _snprintf
#define RTI_STRCASECMP      _stricmp
#define RTI_STRNCASECMP     _strnicmp
#define RTI_STRTOULL        _strtoui64
#define RTI_STRTOUL         strtoul
#else
#define RTI_SNPRINTF        snprintf
#define RTI_STRCASECMP      strcasecmp
#define RTI_STRNCASECMP     strncasecmp
#define RTI_STRTOULL        strtoull
#define RTI_STRTOUL         strtoul
#endif

// ***************************************************************************
// ** Private constants                                                     **
// ***************************************************************************
static const std::string STRING_TRUE("true");
static const std::string STRING_FALSE("false");
static const int MAX_TEMP_PROPERTY_BUFFER = 64;


// ***************************************************************************
// ** ValueMapImpl                                                          **
// ***************************************************************************
#ifdef RTI_WIN32
    class ValueMapImpl: public stdext::hash_map<std::string, std::string, stdext::hash_compare<std::string> > {
    };
#else
    class ValueMapImpl: public __gnu_cxx::hash_map<std::string, std::string, __gnu_cxx::hash<std::string> > {
    };
#endif


// ***************************************************************************
// ** Private static functions                                              **
// ***************************************************************************
// throws not_found if the key is not defined in map
inline static const std::string & _getKey(ValueMapImpl * map, const std::string &key) {
    ValueMapImpl::const_iterator iter = map->find(key);
    if (iter == map->end()) {
        char buf[128];
        RTI_SNPRINTF(buf, 128, "Key not found in ValueMap: %s", key.c_str());
        throw not_found(std::string(buf));
    }
    return iter->second;
}


inline static char * _trim_line(char *ptr) {
    // Strip spaces at the end of the line
    char *temp = ptr + strlen(ptr) - 1;
    while (temp > ptr && isspace(*temp)) { *temp = '\0'; --temp; }
    
    // Strip spaces at the beginning of the line
    while (*ptr && isspace(*ptr)) ++ptr;
    return ptr;
}


// Returns false if the line doesn't contain the '=' sign (invalid format)
inline static bool _insertLineInValueMap(ValueMap *map, char *line) {
    char *sep, *key, *val;
    sep = strchr(line, '=');
    if (sep == NULL) {
        // Invalid line
        return false;
    }
    *sep = '\0';
    key = line;
    val = sep+1;
    // Strip spaces before and after key and val
    key = _trim_line(key);
    val = _trim_line(val);
    map->set_string(std::string(key), std::string(val));
    return true;
}





// ***************************************************************************
// ** ValueMap static functions                                             **
// ***************************************************************************
void ValueMap::parse_as_bool(const std::string &valueIn, bool *valueOut) {
    if ( (valueIn[0] == '1' && valueIn.size() == 1) || !strcmp(valueIn.c_str(), STRING_TRUE.c_str())) {
        if (valueOut) *valueOut = true;
        return;
    }
    if ( (valueIn[0] == '0' && valueIn.size() == 1) || !strcmp(valueIn.c_str(), STRING_FALSE.c_str())) {
        if (valueOut) *valueOut = false;
        return; 
    }
    std::ostringstream os;
    os << "unable to convert value to bool: " << valueIn;
    std::cout << "ValueMap::parse_as_bool: " << os.str() << std::endl;
    throw invalid_type(os.str());
}

// ---------------------------------------------------------------------------
void ValueMap::parse_as_int(const std::string &valueIn, int *valueOut) {
    char *ptr = NULL;
    long retVal = strtol(valueIn.c_str(), &ptr, 10);
    if (*ptr) { 
        std::ostringstream os;
        os << "unable to convert value to int: " << valueIn;
        std::cout << "ValueMap::parse_as_int: " << os.str() << std::endl;
        throw invalid_type(os.str());
    }
    if (valueOut) *valueOut = (int)retVal;
}

// ---------------------------------------------------------------------------
void ValueMap::parse_as_uint(const std::string &valueIn, unsigned int *valueOut) {
    char *ptr = NULL;
    unsigned long retVal = strtoul(valueIn.c_str(), &ptr, 10);
    if (*ptr) { 
        std::ostringstream os;
        os << "unable to convert value to int: " << valueIn;
        std::cout << "ValueMap::parse_as_uint: " << os.str() << std::endl;
        throw invalid_type(os.str());
    }
    if (valueOut) *valueOut = (unsigned int)retVal;
}


// ---------------------------------------------------------------------------
void ValueMap::parse_as_uint_gt0(const std::string &valueIn, unsigned int *valueOut) {
    unsigned int temp;
    parse_as_uint(valueIn, &temp);
    
    if (temp == 0) {
        std::ostringstream os;
        os << "unable to convert value to int_gt0: " << valueIn;
        std::cout << "ValueMap::parse_as_uint_gt0: " << os.str() << std::endl;
        throw invalid_type(os.str());
    }
    if (valueOut) *valueOut = temp;
}



// ---------------------------------------------------------------------------
void ValueMap::parse_as_uint_gte0(const std::string &valueIn, unsigned int *valueOut) {
    unsigned int temp;
    parse_as_uint(valueIn, &temp);
    
    if (valueOut) *valueOut = temp;
}

// ---------------------------------------------------------------------------
void ValueMap::parse_as_double(const std::string &valueIn, double *valueOut) {
    char *ptr = NULL;
    double retVal = (double)strtod(valueIn.c_str(), &ptr);
    if (*ptr) { 
        std::ostringstream os;
        os << "unable to convert value to double: " << valueIn;
        std::cout << "ValueMap::parse_as_double: " << os.str() << std::endl;
        throw invalid_type(os.str());
    }
    if (valueOut) *valueOut = retVal;
}

// ---------------------------------------------------------------------------
void ValueMap::parse_as_duration(const std::string &valueIn, DDS_Duration_t *valueOut) {
    if (valueIn == "I") {
        if (valueOut) *valueOut = DDS_DURATION_INFINITE;
        return;
    }

    char *ptr = NULL;
    unsigned long long usec = RTI_STRTOULL(valueIn.c_str(), &ptr, 10);
    if (*ptr) {
        std::ostringstream os;
        os << "unable to convert value to duration: " << valueIn;
        std::cout << "ValueMap::parse_as_duration: " << os.str() << std::endl;
        throw invalid_type(os.str());
    }
    if (valueOut) {
        valueOut->sec = (DDS_Long)(usec / 1000000ull);
        valueOut->nanosec = (DDS_UnsignedLong)((usec % 1000000ull) * 1000ull);
    }
}





// ***************************************************************************
// ** ValueMap Methods                                                      **
// ***************************************************************************

ValueMap::ValueMap():
    _theImpl(new ValueMapImpl()),
    _isReadOnly(false) {
}

// ---------------------------------------------------------------------------
ValueMap::ValueMap(const ValueMap &ref):
    _theImpl(new ValueMapImpl(*ref._theImpl)),
    _isReadOnly(ref._isReadOnly) { }

// ---------------------------------------------------------------------------
ValueMap::~ValueMap() {
    if (_theImpl != NULL) {
        delete _theImpl;
    }
}

// ---------------------------------------------------------------------------
ValueMap & ValueMap::operator=(const ValueMap &ref) {
    if (this != &ref) {
        if (_theImpl != NULL) {
            // Delete the current map
            delete _theImpl;
        }
        _theImpl = new ValueMapImpl(*ref._theImpl);
        _isReadOnly = ref._isReadOnly;
    }
    return *this;
}

// ---------------------------------------------------------------------------
ValueMap ValueMap::clone() const {
    ValueMap retVal;
    *(retVal._theImpl) = *_theImpl;
    return retVal;
}

// ---------------------------------------------------------------------------
void ValueMap::clone_to(ValueMap &map) const {
    map._isReadOnly = false;
    *(map._theImpl) = *_theImpl;
}

// ---------------------------------------------------------------------------
void ValueMap::merge_to(ValueMap &map) const {
     map._isReadOnly = false;
     for (ValueMapImpl::const_iterator iter=_theImpl->begin(); 
          iter!=_theImpl->end(); 
          ++iter) {
         (*(map._theImpl))[iter->first] = iter->second;
     }
}

// ---------------------------------------------------------------------------
bool ValueMap::is_set(const std::string &key) const {
    return _theImpl->find(key) != _theImpl->end();
}

// ---------------------------------------------------------------------------
bool ValueMap::unset(const std::string &key) {
    if (_isReadOnly) throw read_only(key);
    
    ValueMapImpl::iterator iter = _theImpl->find(key);
    if (iter != _theImpl->end()) {
        _theImpl->erase(iter);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
void ValueMap::set_bool(const std::string &key, bool value) {
    if (_isReadOnly) throw read_only(key);
    (*_theImpl)[key] = (value ? STRING_TRUE : STRING_FALSE);
}

// ---------------------------------------------------------------------------
void ValueMap::set_int(const std::string &key, int value) {
    if (_isReadOnly) throw read_only(key);
    char buf[MAX_TEMP_PROPERTY_BUFFER];
    RTI_SNPRINTF(buf, MAX_TEMP_PROPERTY_BUFFER, "%d", value);
    (*_theImpl)[key] = std::string(buf);
}

// ---------------------------------------------------------------------------
void ValueMap::set_string(const std::string &key, const std::string &value) {
    if (_isReadOnly) throw read_only(key);
    (*_theImpl)[key] = value;
}
    
// ---------------------------------------------------------------------------
void ValueMap::set_double(const std::string &key, double value) {
    if (_isReadOnly) throw read_only(key);
    char buf[MAX_TEMP_PROPERTY_BUFFER];
    RTI_SNPRINTF(buf, MAX_TEMP_PROPERTY_BUFFER, "%g", value);
    (*_theImpl)[key] = buf;
}

// ---------------------------------------------------------------------------
void ValueMap::set_duration(const std::string &key, const DDS_Duration_t &value) {
    if (_isReadOnly) throw read_only(key);
    char buf[MAX_TEMP_PROPERTY_BUFFER];
    if (value.sec== DDS_DURATION_INFINITY_SEC && value.nanosec == DDS_DURATION_INFINITY_NSEC) {
        // Represent infinite duration with "I" for now.
        RTI_SNPRINTF(buf, MAX_TEMP_PROPERTY_BUFFER, "I");
    } else {
        unsigned long long usec = value.sec * 1000000ull + value.nanosec / 1000ull;
        // Store as string in microsecond resolution.
        RTI_SNPRINTF(buf, MAX_TEMP_PROPERTY_BUFFER, "%llu", usec);
    }
    (*_theImpl)[key] = buf;
}

// ---------------------------------------------------------------------------
bool ValueMap::get_bool(const std::string &key) const {
    bool tempVal;
    parse_as_bool(_getKey(_theImpl, key), &tempVal);
    return tempVal;
}


// ---------------------------------------------------------------------------
unsigned int ValueMap::get_uint(const std::string &key) const {
    unsigned int tempVal;
    parse_as_uint(_getKey(_theImpl, key), &tempVal);
    return tempVal;
}

// ---------------------------------------------------------------------------
int ValueMap::get_int(const std::string &key) const {
    int tempVal;
    parse_as_int(_getKey(_theImpl, key), &tempVal);
    return tempVal;
}

// ---------------------------------------------------------------------------
const std::string &ValueMap::get_string(const std::string &key) const {
    return _getKey(_theImpl, key);
}

// ---------------------------------------------------------------------------
double ValueMap::get_double(const std::string &key) const {
    double tempVal;
    parse_as_double(_getKey(_theImpl, key), &tempVal);
    return tempVal;
}

// ---------------------------------------------------------------------------
void ValueMap::get_duration(const std::string &key, DDS_Duration_t &durationOut) const {
    parse_as_duration(_getKey(_theImpl, key), &durationOut);
}



// ---------------------------------------------------------------------------
bool ValueMap::validate_set(const Validator &validator) const {
    for (ValueMapImpl::const_iterator iter = _theImpl->begin(); iter != _theImpl->end(); ++iter) {
        if (!validator.validate(iter->first, iter->second)) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
unsigned int ValueMap::size() const {
    return (unsigned int)_theImpl->size();
}

// ---------------------------------------------------------------------------
void ValueMap::clear() {
    _theImpl->clear();
}

// ---------------------------------------------------------------------------
void ValueMap::serialize_to(std::ostream &os) const {
    for (ValueMapImpl::const_iterator iter = _theImpl->begin(); 
         iter != _theImpl->end(); 
         ++iter) {
        os << iter->first << '=' << iter->second << std::endl;
    }
}


void ValueMap::deserialize_from(std::istream &is) {
    char line[MAX_LINE];
    while(!is.eof()) {
        is.getline(line, MAX_LINE);
        _insertLineInValueMap(this, line);
    }
}


// ***************************************************************************
// ** QosProfile                                                            **
// ***************************************************************************
const QosProfile::QosProfileName QosProfile::ANONYMOUS_NAME("__anonname__");
const QosProfile QosProfile::EMPTY_PROFILE(ANONYMOUS_NAME);


// ***************************************************************************
// ** QosDictionaryImpl                                                     **
// ***************************************************************************
#ifdef RTI_WIN32
    typedef stdext::hash_map<std::string, QosProfile, stdext::hash_compare<std::string> > _QosProfileMap;
#else
    typedef __gnu_cxx::hash_map<std::string, QosProfile, __gnu_cxx::hash<std::string> >  _QosProfileMap;
#endif

class QosDictionaryImpl: public _QosProfileMap {
public:
    QosDictionaryImpl(): _QosProfileMap() {}
    
    void read_from_stream(std::istream &is);
};



// ***************************************************************************
// ** QosDictionary Methods                                                 **
// ***************************************************************************

QosDictionary::QosDictionary():
    _theImpl(new QosDictionaryImpl()) { 
}

// ---------------------------------------------------------------------------
QosDictionary::QosDictionary(const QosProfile &profile):
    _theImpl(new QosDictionaryImpl()) {
    add(profile);
}

// ---------------------------------------------------------------------------
QosDictionary::QosDictionary(const std::string &source): _theImpl(NULL) {
    // Use auto_ptr so exceptions won't cause memory leaks
    std::auto_ptr<QosDictionaryImpl> impl(new QosDictionaryImpl());
    const char * fName = source.c_str();    
    std::ifstream fs;
    fs.open(fName);
    if (!fs.is_open()) {
        throw not_found(fName);
    }
    impl->read_from_stream(fs);
    fs.close();
    _theImpl = impl.release();
}

// ---------------------------------------------------------------------------
QosDictionary::QosDictionary(std::istream &is): _theImpl(NULL) {
    // Use auto_ptr so exceptions won't cause memory leaks
    std::auto_ptr<QosDictionaryImpl> impl(new QosDictionaryImpl());
    impl->read_from_stream(is);
    _theImpl = impl.release();
}

  
// ---------------------------------------------------------------------------
QosDictionary::~QosDictionary() {
    if (_theImpl != NULL) {
        delete _theImpl;
    }
}

// ---------------------------------------------------------------------------
void QosDictionary::add(const QosProfile &profile) {
    (*_theImpl)[profile.get_name()] = profile;
}

// ---------------------------------------------------------------------------
QosProfile *QosDictionary::get(const QosProfile::QosProfileName &name) const {
    QosDictionaryImpl::iterator iter = _theImpl->find(name);
    if (iter == _theImpl->end()) {
        std::cout << "QosDictionary::get: profile not found: " << name << std::endl;
        throw not_found(name);
    }
    return &iter->second;
}
    
// ---------------------------------------------------------------------------
bool QosDictionary::remove(const QosProfile::QosProfileName &name) {
    QosDictionaryImpl::iterator iter = _theImpl->find(name);
    if (iter == _theImpl->end()) {
        return false;
    }
    _theImpl->erase(iter);
    return true;
}

// ---------------------------------------------------------------------------
QosDictionary QosDictionary::clone() const {
    QosDictionary retVal;

    for (QosDictionaryImpl::iterator iter = _theImpl->begin(); iter != _theImpl->end(); ++iter) {
        retVal.add(iter->second.clone());
    }
    return retVal;
}


// ---------------------------------------------------------------------------
unsigned int QosDictionary::size() const {
    return (unsigned int)_theImpl->size();
}


// ***************************************************************************
// ** QosDictionaryImpl Methods                                             **
// ***************************************************************************


void QosDictionaryImpl::read_from_stream(std::istream &is) {
    // Reads line by line a file in the following format:
    //      [name]
    //      key = value
    //      key = value
    //      [name]
    //      ...
    // Comments starts with character '#' or ';' until the end of the line
    QosProfile profile; // The first one is all anonymous name
    const char * const METHOD_NAME("QosDictionaryImpl::read_from_stream");
    char line[MAX_LINE];
    char *ptr;

    for (int lineCount = 1; !is.eof(); ++lineCount) {
        is.getline(line, MAX_LINE);

        // Remove comments
        ptr = line;
        while (*ptr) {
            if (*ptr == '#' || *ptr == ';') {
                *ptr ='\0';
                break;
            }
            ++ptr;
        }

        // Strip spaces before and after
        ptr = _trim_line(line);
        
        // Remove empty lines
        if (ptr[0] == '\0') {
            continue;
        }
        
         // Is a profile definition?
        if (ptr[0] == '[' && ptr[strlen(ptr)-1] == ']') {
            if (strlen(ptr) <= 2) {
                std::ostringstream os;
                os << "invalid profile name on line " << lineCount;
                std::cout << METHOD_NAME << ": " << os.str() << std::endl;
                throw format_error(os.str());
            }
            // Removes the '[' and ']' from the line
            ++ptr;
            ptr[strlen(ptr)-1] = '\0';
            ptr = _trim_line(ptr);
            
            // Insert current profile in map
            if (profile.get_name() != QosProfile::ANONYMOUS_NAME) {
                _QosProfileMap::operator[](profile.get_name()) = profile;
            }
            
            // Create the new profile
            profile = QosProfile(QosProfile::QosProfileName(ptr));
            continue;
        }
        
        if (_insertLineInValueMap(&profile, ptr) == false) {
            std::ostringstream os;
            os << "invalid declaration, missing '=' separator on line " << lineCount;
            std::cout << METHOD_NAME << ": " << os.str() << std::endl;
            throw format_error(os.str());
        }
    } // end of file
    
    // Finally store the last profile
    if (profile.get_name() != QosProfile::ANONYMOUS_NAME) {
        // Don't add if is the default one
        _QosProfileMap::operator[](profile.get_name()) = profile;
    }
}

#ifdef RTI_WIN32
#pragma warning(pop)
#endif







