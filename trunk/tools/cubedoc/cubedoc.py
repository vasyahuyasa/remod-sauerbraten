#!/usr/bin/python3
"""
Remod Cubescript documentation generator
Looks through C++ / CubeScript source files or additional text files, finds and parses documentation comments for functions, events and variables available in cube scripts
Generates html help files based on searched results and specified html templates
Requires Python 3

=====================
Usage: python3 cubedoc.py template_dir output_dir source_dir1 source_dir2 ...

template_dir is the directory where html templates will be searched
output_dir is the directory for generated html help files to be placed in
source_dir is the directory with remod-sauebraten sources and custom cubescripts

=====================
PARSING FUNCTIONS


.cs/.cfg file:

/////////////////////////////////////
// Function description
// @group groupOfFunction
// @arg1 (int) description of the 1st integer argument 
// @arg2 (float) description of the 2nd float argument
// @arg3 description of the 3rd string argument
// @return description of function's result
// @example test_function 0 3.1415 "string param"
// Another line of description
test_function = [

Documentation comment should start with at least 3 backslashes ///
if @group is not specified, the default "SERVER" group will be used


.cpp/.h file:

/**
 * Function description
 * @group group
 * @arg1 (int) description of the 1st integer argument 
 * @arg2 (float) description of the 2nd float argument
 * @arg3 (string) description of the 3rd string argument
 * @arg4 description of the 4th string argument
 * @return what function returns
 * @example test_function 0 3.1415 "string param" "string param2"
 * another description line
 * and one more
 */
COMMANDN(test_function, some_c_function_to_process, "ifss");

Documentation comment should start with /**
if @group is not specified, the default "SYSTEM" group will be used
Specifying types in @arg is not required, by default types are determined from context (types string from COMMANDN is used)

If function, which is defined by COMMAND, has no documentation comments, it will be parsed with empty description fields


.txt file:

@function test_function
@group group
@arg1 (int) description of the 1st integer argument 
@arg2 (float) description of the 2nd float argument
@arg3 description of the 3rd string argument
@return what function returns
@example test_function 0 3.1415 "string param" 
Function description

.txt files with such content could be used, for example, to store help description on "system" functions to don't write additional text to sauerbraten core files
Default group: "SERVER"



=====================
PARSING VARIABLES


.cs/.cfg file:

/////////////////////////////////////
// Variable description
// @group groupOfVariable
// @type int
// @min 0
// @max 100
// @cur 10
// @access default
test_variable = somtehing

@type is the type of variable and could be: int, float, string
@min is the minimal allowed value for int and float variables. It's ignored for strings
@max --"-- maximal --"--
@cur is the current (default) variable value
@behaviour is the access of variable, could be: default, readonly, persist, override. Connected with cubescript engine actions
Default group: "SERVER"


.cpp/.h file:

/**
 * Variable description
 * @group groupOfVariable
 * @type int
 * @min 0
 * @max 100
 * @cur 10
 * @behaviour persist
 */
VAR(var_name, 0, 10, 100)

@type, @min, @max, @cur, @behaviour are not required, they are determined from macros VAR
I think it's a bad practice to comment variables (and events -- see below) in .cpp, so you can use .txt


.txt file:

@variable var_name
@group groupOfVariable
@type int
@min 0
@max 100
@cur 10
@access default
Variable description

=====================
PARSING EVENTS

Script parses .cpp/.h files finding events call ( remod::onevent("eventname", "ifs"  ) and gathers event's name and parameters types   
Description of event could be stored in .txt:

@event eventname
@group groupOfVariable
@arg1 (int) integer argument
@arg2 (float) float argument
@arg3  string argument
@greedy
Description

@greedy shows that event handler is "greedy": by returning FALSE (0) it stops processing of further event handlers  
default group: "EVENT"

=====================

WARNING! THIS SCRIPT HAS A LOT OF CRUTCHS! YOU EYES MAY FLOW OUT

@author: oramahmaalhur
@version 0.2
"""

import os
import os.path
import re
import sys
from functools import reduce
from datetime import datetime

###############################################################################
###### Some data structures 
###############################################################################

class ItemsContainer():
    def __init__(self):
        self._items = {}
        
    def getItem(self, cls, name, *args, **kw):
        if not self._items.get(cls):
            self._items[cls] = {}
        if not self._items[cls].get(name):
            self._items[cls][name] = cls(name, *args, **kw)
        return self._items[cls][name]
    
    def getItems(self, cls):
        return self._items[cls]
        
    def getItemsList(self, cls):
        return [x[1] for x in sorted(self._items[cls].items(), key=lambda x: x[0])]

itemsContainer = ItemsContainer()

# Abstract container
class AbstractNamedItem(object):
    def __init__(self, name):
        self._name = name
        
    def getName(self):
        return self._name
    
    def setName(self, name):
        self._name = name
    
    @classmethod
    def getItem(cls, name, *args, **kw):
        return itemsContainer.getItem(cls, name, *args, **kw)
    
    @classmethod
    def getItems(cls):
        return itemsContainer.getItems(cls)
        
    @classmethod
    def getItemsList(cls):
        return itemsContainer.getItemsList(cls)

# Group of functions, variables, events
class Group(AbstractNamedItem):
    SYSTEM = "system"  # default group for "system" cubescript functions (those ones, which came from sauerbraten code and have no documentation comments).
    SERVER = "server"  # default group for cubescript functions from remod source files (used if no group is specified)
    EVENT = "event" 
    
    def __init__(self, name):
        super().__init__(name)
        self._functions = [] 
        self._events = [] 
        self._variables = []

    def addFunction(self, fn):
        self._functions.append(fn)
        
    def addEvent(self, event):
        self._events.append(event)
        
    def addVariable(self, v):
        self._variables.append(v)
    
    def addCodeItem(self, code_item):
        if isinstance(code_item, Function):
            self.addFunction(code_item)
        elif  isinstance(code_item, Variable):
            self.addVariable(code_item)
        elif isinstance(code_item, Event):
            self.addEvent(code_item)
    
    def sortCodeItems(self, values_map):
        return sorted(values_map, key=lambda x: x.getName())
    
    def getFunctionsList(self):
        return self.sortCodeItems(self._functions)
    
    def hasFunctions(self):
        return len(self._functions) > 0
    
    def getVariablesList(self):
        return self.sortCodeItems(self._variables)
    
    def hasVariables(self):
        return len(self._variables) > 0
    
    def getEventsList(self):
        return self.sortCodeItems(self._events)
        
    def hasEvents(self):
        return len(self._events) > 0
        
    def __str__(self):
        return "[Group: %s]" % (self._name)
        

# Argument of function
class Argument:
    INT = 'int'
    STRING = 'string'
    FLOAT = 'float'
    TYPE_NAMES = {'i': INT, 's': STRING, 'C': STRING, 'f': FLOAT, 'V': STRING}
    
    def __init__(self, name = None, position = None, type = None, description = None):
        self._name = name
        self._type = type
        self._position = position
        self._description = description
    
    def getName(self):
        return self._name
    
    def setName(self, name):
        self._name = name
    
    def getType(self):
        return self._type
    
    def setType(self, type):
        self._type = type
        
    def setTypeChar(self, type_char):
        self._type = Argument.TYPE_NAMES.get(type_char)
    
    def getPosition(self):
        return self._position
    
    def setPosition(self, position):
        self._position = position
    
    def getDescription(self):
        return self._description
    
    def setDescription(self, description):
        self._description = description

    def __str__(self):
        return "(%s : %s : %s)" % (self._name, self._type, self._description)


# Abstract Code Item (function, variable, event)

class AbstractCodeItem(AbstractNamedItem):
    def __init__(self, name=None, source_file=None, group_name = None, description=None):
        super().__init__(name)
        self._source_file = source_file
        self._description = description
        
        self._group = None
        
        if group_name:
            self.setGroupName(group_name)
            
    def setGroupName(self, group_name):
        self._group = Group.getItem(group_name)
        self._group.addCodeItem(self)
    
    def getGroup(self):
        return self._group
    
    def setSourceFile(self, source_file):
        self._source_file = source_file
    
    def getSourceFile(self):
        return self._source_file
    
    def setDescription(self, description):
        self._description = description
        
    def addDescriptionLine(self, line):
        if (self._description):
             self._description += '\n'
        self._description += line
    
    def getDescription(self):
        return self._description
    

# Abstract code item with arguments (function, event)
class AbstractCodeItemWithArgumens(AbstractCodeItem):
    def __init__(self, name=None, source_file=None, group_name = None, description=None, argument_types = []):
        super().__init__(name, source_file=source_file, group_name=group_name, description=description)
        self._arguments = {}
        
        if argument_types:
            self.setArgumentTypes(argument_types)
    
    def setArgumentTypes(self, argument_types):
        for i in range(len(argument_types)):
            self.getArgument(i).setTypeChar(argument_types[i])
            self.getArgument(i).setName('arg%d' % (i+1)) 
        
    def getArgument(self, position):
        position = int(position)
        if (self._arguments.get(position) == None):
            self._arguments[position] = Argument(name = 'arg%d' % (position+1), position = position)
        return self._arguments[position]
        
    def getArgumentsList(self):
        return [x[1] for x in sorted(self._arguments.items(), key=lambda x: x[0])]
        

# Container of information about function
class Function(AbstractCodeItemWithArgumens):
    def __init__(self, name = None, source_file = None, group_name = None, description = '', argument_types = [], returned_value = None, example = None):
        super().__init__(name, source_file=source_file, group_name=group_name, description=description, argument_types=argument_types)
        
        self._returned_value = returned_value
        self._example = example
                
    
    def setReturnedValue(self, returned_value):
        self._returned_value = returned_value
        
    def getReturnedValue(self):
        return self._returned_value
    
    def setExample(self, example):
        self._example = example
        
    def getExample(self):
        return self._example
        
    def __str__(self):
        s = """%s
       file: %s
      group: %s
  arguments: %s
     return: %s
    example: %s
description: %s\n""" % (
                self._name, 
                self._source_file,
                self._group.getName(),
                reduce(lambda arg, x: str(arg)+str(x)+"   ", self._arguments.values(), ""),
                self._returned_value,
                self._example,
                self._description)
        return s

    @classmethod
    def setDefaults(cls):
        for func in cls.getItems().values():
            if not func.getGroup():
                func.setGroupName(Group.SERVER)

#Container of information about event
class Event(AbstractCodeItemWithArgumens):
    def __init__(self, name=None, source_file=None, group_name = None, description='', argument_types=[], greedy=False):
        super().__init__(name, source_file=source_file, group_name=group_name, description=description, argument_types=argument_types)
        
        # if greedy is true, the event can be eaten by the current event handler
        # so event handler can just return false and the further event processing will be stopped
        self._greedy = greedy 
    
    def getGreedy(self):
        return self._greedy
    
    def setGreedy(self, greedy):
        self._greedy = greedy
        
    def __str__(self):
        s = """%s
       file: %s
      group: %s
     greedy: %s
  arguments: %s
description: %s\n""" % (
                self._name, 
                self._source_file,
                self._group.getName(),
                self._greedy,
                reduce(lambda arg, x: str(arg)+str(x)+"   ", self._arguments.values(), ""),
                self._description)
        return s

    def __repr__(self):
        return self.__str__()
    
    @classmethod
    def setDefaults(cls):
        for evt in cls.getItems().values():
            if not evt.getGroup():
                evt.setGroupName(Group.EVENT)

#Container of info about variable
class Variable(AbstractCodeItem):
    
    INT = 'int'
    FLOAT = 'float'
    STRING = 'string'
    HEX = 'hex'
    
    DEFAULT = 'read/write'
    PERSIST = 'persist'
    OVERRIDE = 'override'
    READONLY = 'readonly'
    
    TYPE_CHARACTERS = {'' : INT, 'I' : INT, 'F' : FLOAT, 'H' : HEX, 'S' : STRING}
    access_VALUES = {'IDF_READONLY': READONLY, 'IDF_OVERRIDE': OVERRIDE, 'N': DEFAULT, '': DEFAULT, 'NP': PERSIST, 'P': PERSIST, 'NR': OVERRIDE, 'R': OVERRIDE}
    def __init__(self, name=None, source_file=None, group_name = None, description='', type=None, min_value=None, cur_value=None, max_value=None, access=None):
        super().__init__(name, source_file=source_file, group_name=group_name, description=description)
        self._type = type
        self._cur_value = cur_value
        self._min_value = min_value
        self._max_value = max_value
        self._access = access
        
    def getType(self):
        return self._type
    
    def setType(self, type):
        self._type = type
    
    def getMinValue(self):
        return self._min_value
    
    def setMinValue(self, min_value):
        self._min_value = min_value
        
    def getCurValue(self):
        return self._cur_value
    
    def setCurValue(self, cur_value):
        self._cur_value = cur_value
    
    def getMaxValue(self):
        return self._max_value
    
    def setMaxValue(self, max_value):
        self._max_value = max_value
        
    def getAccess(self):
        return self._access
        
    def setAccess(self, access):
        self._access = access

    def __str__(self):
        return """%s
       file: %s
      group: %s
       type: %s
        min: %s
        cur: %s
        max: %s
     access: %s
description: %s\n""" % (self.getName(), self.getSourceFile(), self.getGroup(), self.getType(), self.getMinValue(), self.getCurValue(), self.getMaxValue(), self.getAccess(), self.getDescription())

    @classmethod
    def setDefaults(cls):
        for var in cls.getItems().values():
            if not var.getType():
                var.setType(Variable.STRING)
            if not var.getGroup():
                var.setGroupName(Group.SYSTEM)
    
###############################################################################
###### Parsers
###############################################################################

# Abstract parser class
class AbstractParser:
    
    def __init__(self, base_dir):
        self._base_dir = base_dir
    
    # to delete whitespaces
    REG_LINE_WHITESPACES = re.compile(r'([ \t]*\r?\n[ \t]*)')

    #to delete empty lines 
    REG_EMPTY_LINE = re.compile(r'(\n{2,})') 
    
    #  Read file and prettify its content
    def readFileContent(self, file):
        cnt = ""
        f = None
        try:
            f = open(os.path.join(self._base_dir, file), "r")
            cnt = f.read()
        except:
            pass
        finally:
            if f:
                f.close()
        
        if not cnt: 
            return None
        
        cnt = cnt.replace('\r', '')
        cnt = AbstractParser.REG_LINE_WHITESPACES.sub(r'\n', cnt) #prettifying source code
        cnt = AbstractParser.REG_EMPTY_LINE.sub(r'\n', cnt)
        
        return cnt
    
    def stripLine(self, line):
        line = line.strip()
        line = re.sub('^\/\/+\s*', "", line)
        line = re.sub("^\*\/\s*", "", line)
        line = re.sub("^\*+\s*", "", line)
        line = re.sub("^\/\*+\s*", "", line)
        return line
    
    def searchAndSetValue(self, line, value, function):
        r = re.search('^@'+value+' ?(.*?)$', line, re.I)
        if r and r.group(1):
            function(r.group(1))
            return True
        return False
    
    def searchAndSetArguments(self, line, code_item):
        r = re.search('^@arg(\d+) ?(?:\((\w+)\))? ?(.*?)$', line, re.I)
        if r and r.group(1):
            argument = code_item.getArgument(int(r.group(1))-1)
            argument.setName('arg'+r.group(1))
            if r.group(2):
                argument.setType(r.group(2))
            elif not argument.getType():
                argument.setType(Argument.STRING)
            argument.setDescription(r.group(3))
            return True
        return False      
    
    # Parse function documentation comment, set data into func
    def parseFunctionComment(self, comment_lines, func):
        """
        Parse comment to extract documentation
        /**
         * function description
         * @name my_function_name //name of function. If not defined uses system name (defined in CPP as COMMAND(function_name... or in CS as function_name = [...)
         * @group group_name //if not defined uses CUSTOM_DEFAULT_GROUP
         * @arg1 argument 1 description //not required
         * @arg2 argument 2 description //not required
         * @return what this returns //not required
         * @example example of usage  //not required
         */
         
         or the same with //  :
         
         // function description
         // @group group_name
         // .....
        """
        
        for line in comment_lines:
            line = self.stripLine(line)
            
            if not line \
                    or self.searchAndSetValue(line, 'name', func.setName) \
                    or self.searchAndSetValue(line, 'group', func.setGroupName) \
                    or self.searchAndSetValue(line, 'example', func.setExample) \
                    or self.searchAndSetValue(line, 'return', func.setReturnedValue) \
                    or self.searchAndSetArguments(line, func):
                continue
            func.addDescriptionLine(line)
        return func
            
    
    def parseVariableComment(self, comment_lines, var):
        for line in comment_lines:
            line = self.stripLine(line)
            if not line \
                    or self.searchAndSetValue(line, 'name', var.setName) \
                    or self.searchAndSetValue(line, 'group', var.setGroupName) \
                    or self.searchAndSetValue(line, 'min', var.setMinValue) \
                    or self.searchAndSetValue(line, 'max', var.setMaxValue) \
                    or self.searchAndSetValue(line, 'cur', var.setCurValue) \
                    or self.searchAndSetValue(line, 'access', var.setAccess) \
                    or self.searchAndSetValue(line, 'type', var.setType):
                continue
            var.addDescriptionLine(line)
        return var
    
    
    def parseEventComment(self, comment_lines, event):
        for line in comment_lines:
            line = self.stripLine(line)
            if not line \
                    or self.searchAndSetValue(line, 'group', event.setGroupName) \
                    or self.searchAndSetArguments(line, event):
                continue
            if re.search('^@greedy', line, re.I):
                event.setGreedy(True)
                continue
            event.addDescriptionLine(line)
            
        return event

# Parse .cpp, .h files 
class CppParser(AbstractParser):
    
    # to find the end of doc comment in cpp file
    REG_FUNCTION_GENERAL = re.compile(r'''
        (?<=\*\/\n)                        # if it has */ before COMMAND
        (COMMANDN?                         # COMMAND or COMMANDN 
            \(\s*[^, \t\r\n]+\s*,          # name of command is placed after ( and before ,
            [^"]*                          # everything between , and " is not interesting
            "[^"]*"                        # types of command's args are placed in quotes 
        ) 
    ''', re.VERBOSE)
    
    # to extract function name and args types 
    REG_FUNCTION_DETAILS = re.compile(r'''
        COMMANDN?                          # COMMAND or COMMANDN 
            \(\s*([^,\s\t\r\n]+)\s*,       # name of command is placed after ( and before ,
            [^"]*                          # everything between , and " is not interesting
            "([^"]*)"                      # types of command's args are placed in quotes  
    ''', re.VERBOSE)
    
    # to find functions with no documentation comments
    REG_FUNCTION_NODESCR = re.compile(r'(?<!\*\/\n)COMMAND\(\s*([^, \t\r\n]+)\s*,[^"]*"([^"]*)"')

    # to find variables
    # _VAR, _HVAR, _FVAR, _IVAR
    REG_VAR_ = re.compile(r'''
        _                                  # should be _VAR, starts with _
        (F|H|I)?                           # float, hex or integer var
        VAR                                # so, just _(F|S|H|I)?VAR
        \(
            ([^,]+),\s*                    # name
            ([^,]+),\s*                    # global
            ([^,]+),\s*                    # min value | current value for String
            ([^,]+),\s*                    # current value | access flag for String 
            ([^,]+),\s*                    # max value  | empty for String
            ([^\)]+)\s*                    # access flag \ empty for String
        \)''', re.VERBOSE)
    
    # VARN, HVARN, FVARN, IVARN
    REG_VARN = re.compile(r'''
        (?<!_)                             # should NOT start with _
        (?<!S)                             # not string! 
        (F|H|I)?                           # float, hex or integer var
        (?<!GET)                           # we don't need GETVAR, 
        (?<!OVERRIDE)                      # nor OVERRIDEVAR macroses
        VAR                                # so, just _(F|S|H|I)?VAR
        (FN|N|NP|NR)                       # access flag
        \(
            ([^,]+),\s*                    # name
            ([^,]+),\s*                    # global
            ([^,]+),\s*                    # min value
            ([^,]+),\s*                    # current value
            ([^,\)]+)                      # max value 
        ''', re.VERBOSE)
    
    # VAR, HVARF, FVARFP, IVARFN
    REG_VAR = re.compile(r'''
        (?<!_)                             # should NOT start with _
        (F|H|I)?                           # float, hex or integer var
        (?<!GET)                           # we don't need GETVAR, 
        (?<!S)                             # not string! 
        (?<!OVERRIDE)                      # nor OVERRIDEVAR macroses
        VAR                                # so, just (F|S|H|I)?VAR
        F?
        (R|P)?                             # access flag
        \(
            ([^,]+),\s*                    # name
            ([^,]+),\s*                    # min value
            ([^,]+),\s*                    # current value
            ([^,\)]+)                      # max value 
        ''', re.VERBOSE)
    
    # _SVAR
    REG_SVAR_ = re.compile(r'''
        _SVAR                              # _SVAR
        \(
            ([^,]+),\s*                    # name
            ([^,]+),\s*                    # global
            ([^,]+),\s*                    # current value
            ([^\)]+)\s*                    # access flag \ empty for String
        \)''', re.VERBOSE)
    
    # SVARN, SVARFN, SVARNP, SVARNR
    REG_SVARN = re.compile(r'''
        (?<!_)                             # should NOT start with _
        (?<!GET)                           # we don't need GETVAR, 
        (?<!OVERRIDE)                      # nor OVERRIDEVAR macroses
        SVAR                                # so, just _(F|S|H|I)?VAR
        (FN|N|NP|NR)                       # access flag
        \(
            ([^,]+),\s*                    # name
            ([^,]+),\s*                    # global
            ([^,\)]+)                      # current value 
        ''', re.VERBOSE)
    
    # SVAR, SVARF, SVARFP, SVARFN
    REG_SVAR = re.compile(r'''
        (?<!_)                             # should NOT start with _
        SVAR                               # so, just SVARF, SVARFP, SVARFN 
        F?
        (R|P)?                             # access flag
        \(
            ([^,]+),\s*                    # name
            ([^,\)]+)\s*                   # current value
        ''', re.VERBOSE)
    
    
    # events
    REG_EVENT = re.compile(r'(if\s*\(\s*|\&\&\s*)?remod::onevent\s*\("([^"]+)", "([^"]+)"')
    
    def __init__(self, *p, **kw):
        AbstractParser.__init__(self, *p, **kw)
    
    def parse(self, path):
        if path == os.path.join('shared', 'command.h'): #it has only definitions, nothing interest
            return
        
        cnt = self.readFileContent(path)
        if not cnt: 
            return
        cnt = cnt.replace("ICOMMAND", "COMMAND") #it doesn't matter for documentation, whether ICOMMAND or COMMAND
        
        #finding commands with documentation
        r = CppParser.REG_FUNCTION_GENERAL.findall(cnt, re.MULTILINE)
        if r: 
            for f in r:
                #extracting name of function and its parameters' types
                r2 = CppParser.REG_FUNCTION_DETAILS.search(f)
                name = r2.group(1)
                arg_types = r2.group(2)
                
                #finding comments
                i = cnt.find(f)
                j = cnt.rfind("/*", 0, i) # /* should always be before */  
                comments = cnt[j:i-1]
                
                func = Function.getItem(name, source_file = path, argument_types=arg_types)
                self.parseFunctionComment(comments.splitlines(), func)                
        
        #finding commands without documentation
        r = CppParser.REG_FUNCTION_NODESCR.findall(cnt, re.MULTILINE)
        if r: 
            for f in r:
                name, arg_types = f
                func = Function.getItem(name, source_file = path, argument_types=arg_types, group_name=Group.SYSTEM)
        
        #finding variables
        for v in CppParser.REG_VAR_.findall(cnt, re.MULTILINE):
            (type, name, glob, min, cur, max, access) = v
            Variable.getItem(name, source_file=path, type=Variable.TYPE_CHARACTERS[type], min_value=min, cur_value=cur, max_value=max, access=Variable.access_VALUES[access])
            
        for v in CppParser.REG_VARN.findall(cnt, re.MULTILINE):
            (type, access, name, glob, min, cur, max) = v
            Variable.getItem(name, source_file=path, type=Variable.TYPE_CHARACTERS[type], min_value=min, cur_value=cur, max_value=max, access=Variable.access_VALUES[access])
            
        for v in CppParser.REG_VAR.findall(cnt, re.MULTILINE):
            (type, access, name, min, cur, max) = v
            Variable.getItem(name, source_file=path, type=Variable.TYPE_CHARACTERS[type], min_value=min, cur_value=cur, max_value=max, access=Variable.access_VALUES[access])

        for v in CppParser.REG_SVAR_.findall(cnt, re.MULTILINE):
            (name, glob, cur, access) = v
            Variable.getItem(name, source_file=path, type=Variable.STRING, cur_value=cur, access=Variable.access_VALUES[access])
            
        for v in CppParser.REG_SVAR.findall(cnt, re.MULTILINE):
            (access, name, cur) = v
            Variable.getItem(name, source_file=path, type=Variable.STRING, cur_value=cur, access=Variable.access_VALUES[access])

        for v in CppParser.REG_SVARN.findall(cnt, re.MULTILINE):
            (type, access, name, glob, cur) = v
            Variable.getItem(name, source_file=path, type=Variable.TYPE_CHARACTERS[type], cur_value=cur, access=Variable.access_VALUES[access])

    
        
        # finding events
        for e in CppParser.REG_EVENT.findall(cnt, re.MULTILINE):
            greedy, name, arg_types = e
            e = Event.getItem(name, source_file = path, argument_types=arg_types, greedy=bool(greedy))
    
# Parse .cs and .cfg files
class CubeScriptParser(AbstractParser):
    
    # to find function definition in cubescript
    REG_FUNCTION_NAME = re.compile('\s*([a-zA-Z_0-9]+)\s*=\s*\[')
    
    # to find variables description in cubescript
    REG_VARIABLE_NAME = re.compile('\s*([a-zA-Z_0-9]+)\s*=\s*[^\[]')
    
    # to find variables in cubescript declared by defaultvalue function
    REG_VARIABLE_NAME2 = re.compile('\s*defaultvalue\s*"([^"]+)"\s*([^$/]+)')
    
    def __init__(self, *p, **kw):
        AbstractParser.__init__(self, *p, **kw)
    
    def parse(self, path):
        """
        Parse cube script to find documentation strings
        @param path: file path
        @return list of dicts
        """
                
        cnt = self.readFileContent(path)
        if not cnt: 
            return
        
        lines = cnt.splitlines()
        length = len(lines)
        i = 0
        while (i < length):
            if lines[i].startswith("///"):
                k = i
                i += 1
                while (i < length) and (lines[i].startswith('//')):
                    i += 1
    
                if (i < length):
                    r = CubeScriptParser.REG_FUNCTION_NAME.search(lines[i])
                    if r and r.group(1):
                        func = Function.getItem(r.group(1), source_file = path)
                        self.parseFunctionComment(lines[k:i], func)
                    else:
                        r = CubeScriptParser.REG_VARIABLE_NAME.search(lines[i])
                        if r and r.group(1):
                            var = Variable.getItem(r.group(1), source_file = path)
                            self.parseVariableComment(lines[k:i], var)
                        else:
                            r = CubeScriptParser.REG_VARIABLE_NAME2.search(lines[i])
                            if r and r.group(1):
                                var = Variable.getItem(r.group(1), source_file = path)
                                var.setCurValue(r.group(2))
                                self.parseVariableComment(lines[k:i], var)
                            else:
                                i += 1
            else:
                i += 1
                


# Parse text file
class TxtParser(AbstractParser):
    def __init__(self, *p, **kw):
        AbstractParser.__init__(self, *p, **kw)
    
    def parse(self, path):
        cnt = ""
        f = None
        try:
            f = open(os.path.join(self._base_dir, path), "r")
            cnt = f.read()
        except:
            pass
        finally:
            if f:
                f.close()
            
        if not cnt: 
            return
        
        lines = cnt.splitlines()
        length = len(lines)
        i = 0
        
        while (i < length):
            line = lines[i]
            
            current_item = None
            if line.startswith("@function "):
                name = line[10:].strip()
                current_item = Function.getItem(name)
            elif line.startswith("@variable "):
                name = line[10:].strip()
                current_item = Variable.getItem(name)
            elif line.startswith("@event "):
                name = line[7:].strip()
                current_item = Event.getItem(name)
            else:
                i += 1
                continue
            
            i += 1
            k = i
            while (i < length) and (len(lines[i].strip()) > 0):
                i += 1
            
            if isinstance(current_item, Function):
                self.parseFunctionComment(lines[k:i], current_item)
            elif isinstance(current_item, Variable):
                self.parseVariableComment(lines[k:i], current_item)
            elif isinstance(current_item, Event):
                self.parseEventComment(lines[k:i], current_item)
                
                
###############################################################################
###### Templates
###############################################################################

class TemplateElement:
    ROOT="ROOT"
    TEXT="TEXT"
    LOOP="LOOP"
    IF="IF"
    TAGS = {"loop": LOOP, "if": IF}
    PARSE_EXPRESSION = re.compile('\$\{([^\}]+)\}')
    PARSE_VARIABLE = re.compile('var="([^"]+)"')
    PARSE_VALUE = re.compile('val="(?:\$\{)?([^\}]+)\}?"')
    
    def __init__(self, parent, type, value):
        self.parent = parent
        self.type = type
        self.value = value
        self.children = []
        
    def add(self, type, value):
        child = TemplateElement(self, type, value)
        self.children.append(child)
        return child
    
    def __str__(self):
        s = '[TemplateElement:'+self.type+'\n'
        s += self.value+"\n"
        for c in self.children:
           if c:
                s += c.__repr__()
        s += ']\n'
        return s

    def __repr__(self):
        return self.__str__()
    
    def _render_children(self, data):
        s = ""
        for c in self.children:
            s += c.render(data)
        return s
    
    #render tree with data
    def render(self, data):
        s = ""
        if self.type == TemplateElement.TEXT:
            s = self.PARSE_EXPRESSION.sub(lambda x: eval(x.group(1), data), self.value)
            
        elif self.type == TemplateElement.LOOP:
            var = self.PARSE_VARIABLE.search(self.value).group(1)
            val = self.PARSE_VALUE.search(self.value).group(1)
            
            loop_items = eval(val, data)
            
            for l in loop_items:
                d = data
                d[var] = l
                s += self._render_children(d)
                
        elif self.type == TemplateElement.IF:
            val = self.PARSE_VALUE.search(self.value).group(1)
            condition = eval(val, data)
            if condition:
                s += self._render_children(data)
        else:
            s += self._render_children(data)
        return s

class Template:
    def __init__(self, template_dir, file, result_dir):
        self._template_path = os.path.join(template_dir, file)
        self._result_path = os.path.join(result_dir, file)
        
        
    def render(self, data):        
        t = open(self._template_path)
        template = t.read()
        t.close()
        
        result = ""
    
        #parse template to find loop and condition tags
        r = re.compile("<(\/?)py:(\w+)\s*([^>]*)>", re.I)
        root = TemplateElement(None, TemplateElement.ROOT, "")
        
        start = 0
        end = 0
        node = root
        for s in r.finditer(template):
            is_closing = s.group(1) == "/"
            start_reg = s.start(0)
            end = s.end(0)
            tag = s.group(2)
            expression = s.group(3)
            
            text = template[start:start_reg]
            node.add(TemplateElement.TEXT, text)
            
            if is_closing:
                node = node.parent
            else:
                node = node.add(TemplateElement.TAGS[tag], expression)
            
            start = end
        
        node.add(TemplateElement.TEXT, template[end:])
        
        result = root.render(data)
        
        t = open(self._result_path, 'w+')
        t.write(result)
        t.close()


class EvenOdd:
    def __init__(self):
        self._c = 0
        
    def __call__(self, evenVal, oddVal):
        self._c += 1
        return evenVal if self._c % 2 else oddVal


###############################################################################
###### Main Class
###############################################################################

class CubeDoc:
    ### extentions of files which can be parsed 
    PARSERS = {'.cpp': CppParser,
               '.h' : CppParser,
               '.cfg': CubeScriptParser,
               '.cs' : CubeScriptParser,
               '.txt': TxtParser
               }

    TMPL_EXTENSION = '.html' # template files

    def extractDocumentation(self, source_dir):
        """
        search recursively for files with specified extensions starting from top_dir
        @param top_dir: root directory to search files
        @return: list of file names
        """
        
        parsers = {}
        
        #search files with these extensions only
        extensions = CubeDoc.PARSERS.keys()
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                for extension in extensions:
                    if file.endswith(extension):
                        source_file = os.path.join(root, file)
                        source_file = source_file.replace(source_dir, '')
                        cls = CubeDoc.PARSERS.get(extension)
                        if not parsers.get(cls):
                            parsers[cls] = cls(source_dir)
                        parsers[cls].parse(source_file)
                        
       
    def renderTemplates(self, tmpl_dir, doc_dir):
        data = {'groups': Group, 
                'functions': Function,
                'variables': Variable,
                'events': Event,
                'nl2br': nl2br,
                'date': datetime.now(),
                'evenOdd': EvenOdd()}
        
        for root, dirs, files in os.walk(tmpl_dir):
            for file in files:
                if file.endswith(CubeDoc.TMPL_EXTENSION):
                     template = Template(root, file, doc_dir)
                     template.render(data)
         
def nl2br(s):
    return s.replace("\n", "<br/>\n")

def usage():
    print("Usage: cubedoc.py template_dir output_dir  source_dir1  source_dir2 ...")

if __name__ == "__main__":
    
    if len(sys.argv) < 3:
        usage()
    else :
        template_dir = sys.argv[1]
        output_dir = sys.argv[2]
        source_dirs = sys.argv[3:]
        cubedoc = CubeDoc()
        for source_dir in source_dirs:
            cubedoc.extractDocumentation(source_dir) 
        
        Function.setDefaults()
        Variable.setDefaults()
        Event.setDefaults()
        

#        for group in Group.getItemsList():
#            print("Group: %s" % group)
#            print("----- FUNCTIONS ------")
#            for func in group.getFunctionsList():
#                print(func)
#            print("----- VARIABLES ------")    
#            for var in group.getVariablesList():
#                print(var)
#            print("----- EVENTS ------")    
#            for event in group.getEventsList():
#                print(event)
#            print("------------------\n\n")
        cubedoc.renderTemplates(template_dir, output_dir)
    
    
# l.d.dlw