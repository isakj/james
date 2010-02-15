/* 
 * File:   Class.cpp
 * Author: tjoppen
 * 
 * Created on February 14, 2010, 4:20 PM
 */

#include "Class.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

Class::Class(FullName name, ClassType type) : name(name), type(type), 
        base(NULL), isBasic(false) {
}

Class::Class(FullName name, ClassType type, Class *base) : name(name),
        type(type), base(base), isBasic(false) {
}

Class::~Class() {
}

void Class::addMember(string name, Member memberInfo) {
    if(members.find(name) != members.end())
        throw runtime_error("Member " + name + " defined more than once in " + this->name.second);

    cout << this->name.second << " got " << memberInfo.type.first << ":" << memberInfo.type.second << " " << name << ". Occurance: ";

    if(memberInfo.maxOccurs == UNBOUNDED) {
        cout << "at least " << memberInfo.minOccurs;
    } else if(memberInfo.minOccurs == memberInfo.maxOccurs) {
        cout << "exactly " << memberInfo.minOccurs;
    } else {
        cout << "between " << memberInfo.minOccurs << "-" << memberInfo.maxOccurs;
    }

    cout << endl;

    members[name] = memberInfo;
}

/**
 * Default implementation of generateAppender()
 */
string Class::generateAppender() const {
    ostringstream oss;
    
    for(std::map<std::string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        string name = it->first;
        string nodeName = name + "Node";

        oss << "DOMElement *" << nodeName << " = node->getOwnerDocument()->createElement(X(\"" << name << "\"));" << endl;
        oss << it->second.cl->generateNodeSetter(name, nodeName) << endl;
    }

    return oss.str();
}

string Class::generateNodeSetter(string memberName, string nodeName) const {
    return memberName + "->appendChildren(" + nodeName + ");";
}

string Class::generateParser() const {
    return "";
}

string Class::generateMemberSetter(string memberName, string nodeName) const {
    return memberName + "->parseNode(" + nodeName + ");";
}

string Class::getClassname() const {
    return name.second;
}

string Class::getClassType() const {
    if(isBasic)
        return getClassname();
    else
        return "boost::shared_ptr<" + name.second + ">";
}

void Class::writeImplementation(ostream& os) const {
    //cout << "writeImplementation()" << endl;
    ClassName className = name.second;

    os << "#include \"" << className << ".h\"" << endl;

    os << "void " << className << "::appendChildren(xercesc::DOMNode *node) const {" << endl;
    os << "}" << endl << endl;
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef _" << className << "_H" << endl;
    os << "#define _" << className << "_H" << endl;

    os << "#include <vector>" << endl;
    os << "#include <boost/shared_ptr.hpp>" << endl;
    os << "#include \"XMLObject.h\"" << endl;

    //non-basic member class prototypes
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++)
        if(!it->second.cl->isBasic)
            os << "class " << it->second.cl->getClassname() << ";" << endl;

    os << "class " << className << " : public james::XMLObject {" << endl;

    //prototypes
    os << "void appendChildren(xercesc::DOMNode *node) const;" << endl;
    os << "void parseNode(xercesc::DOMNode *node) const;" << endl;

    os << "public:" << endl;

    //members
    for(map<string, Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        os << it->second.getType() << " " << it->first << ";" << endl;
    }

    os << "};" << endl;

    os << "#endif //_" << className << "_H" << endl;
}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isRequired() const {
    return minOccurs >= 1;
}

string Class::Member::getType() const {
    //vector if array, regular member otherwise
    if(isArray()) {
        return "std::vector<" + cl->getClassname() + " >";
    } else
        return cl->getClassname();
}
