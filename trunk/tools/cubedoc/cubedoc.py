#!/usr/bin/python3
"""
Remod Cubescript documentation generator
@author: oramahmaalhur
@version 0.1
"""

import os
import os.path
import re

### extentions of files which can be parsed 
CPP_EXTENSIONS = ('.cpp', '.h') # cpp files
CS_EXTENSIONS = ('.cfg', '.cs') # cubescript files

# types of function arguments in cpp
TYPE_NAMES = {'i': 'int', 's': 'string', 'C': 'string'}


def do_search(top_dir):
    """
    search recursively files with specified extensions starting from top_dir
    @param top_dir: root directory to search files
    @return: list of file names
    """
    result = []
    
    #search files with these extensions only
    extensions = CPP_EXTENSIONS + CS_EXTENSIONS
    for root, dirs, files in os.walk(top_dir):
        for file in files:
            for extension in extensions:
                if file.endswith(extension):
                    result.append(os.path.join(root, file))
                    break
    return result
       
       
#########################
    
    
### some precompiled regular expressions

# to delete whitespaces
REG_LINE_WHITESPACES = re.compile(r'([ \t]*\r?\n[ \t]*)')

#to delete empty lines 
REG_EMPTY_LINE = re.compile(r'(\n{2,})') 

def read_file_content(path):
    """
    Read file and prettify its content  
    """
    cnt = ""
    f = open(path, "r")
    try:
        cnt = f.read()
    except:
        pass
    finally:
        f.close()
    
    if not cnt: return None
    cnt = cnt.replace('\r', '')
    cnt = REG_LINE_WHITESPACES.sub(r'\n', cnt) #prettifying source code
    cnt = REG_EMPTY_LINE.sub(r'\n', cnt)
    return cnt
    


#########################

### some precompiled regular expressions

# to find the end of doc comment in cpp file
REG_CPP_COMMAND_GENERAL = re.compile(r'''
    (?<=\*\/\n)                        # if it has */ before COMMAND
    (COMMANDN?                         # COMMAND or COMMANDN 
        \(\s*[^, \t\r\n]+\s*,          # name of command is placed after ( and before ,
        [^"]*                          # everything between , and " is not interesting
        "[^"]*"                        # types of command's args are placed in quotes 
    ) 
''', re.VERBOSE)

# to extract function name and args types 
REG_CPP_COMMAND_DETAILS = re.compile(r'''
    COMMANDN?                          # COMMAND or COMMANDN 
        \(\s*([^, \t\r\n]+)\s*,        # name of command is placed after ( and before ,
        [^"]*                          # everything between , and " is not interesting
        "([^"]*)"                      # types of command's args are placed in quotes  
''', re.VERBOSE)

# to find functions with no documentation comments
REG_CPP_COMMAND_NODESCR = re.compile(r'(?<!\*\/\n)COMMAND\(\s*([^, \t\r\n]+)\s*,[^"]*"([^"]*)"')

def parse_cpp_file(file_path):
    """
    Parse CPP source/header file to find documentation strings
    @param path: file path
    @return: list of dicts
    """
    
    def format_result(name, arg_types, comments=None):
        rs = {'name': name, 'path': file_path}
        
        if comments:
            cmnts = parse_comment(comments.split('\n')) 
            rs.update(cmnts)
        else:
            #if no comment it's system group
            rs.update({'groups': ('system',)})         
            
        if not rs.get('args'):
            rs['args'] = {}
            
        for i in range(len(arg_types)):
            if rs['args'].get(str(i)):
                rs['args'][str(i)].update({'type': TYPE_NAMES.get(arg_types[i])})
            else:
                rs['args'][str(i)] = {'type': TYPE_NAMES.get(arg_types[i])}
        return rs
    
    result = []
    
    cnt = read_file_content(file_path)
    if not cnt: 
        return
    cnt = cnt.replace("ICOMMAND", "COMMAND") #it doesn't matter for documentation, ICOMMAND or COMMAND

    #finding commands with documentation
    r = REG_CPP_COMMAND_GENERAL.findall(cnt, re.MULTILINE)
    if r: 
        for cmd in r:
            #extracting name of function and its parameters' types
            r2 = REG_CPP_COMMAND_DETAILS.search(cmd)
            name = r2.group(1)
            arg_types = r2.group(2)
            
            #finding comments
            i = cnt.find(cmd)
            j = cnt.rfind("/*", 0, i) # /* should always be before */  
            comments = cnt[j:i-1]
            
            result.append(format_result(name, arg_types, comments))
    
    #finding commands without documentation
    r = REG_CPP_COMMAND_NODESCR.findall(cnt, re.MULTILINE)
    if r: 
        for cmd in r:
            name = cmd[0]
            arg_types = cmd[1]

            result.append(format_result(name, arg_types, None))
            
    return result



#########################

# to find function definition in cubescript
REG_CS_FUNCTION_NAME = re.compile('\s*([a-zA-Z_0-9]+)\s*=\s*\[')

def parse_cs_file(file_path):
    """
    Parse cube script to find documentation strings
    @param path: file path
    @return list of dicts
    """
    
    result = []
    
    cnt = read_file_content(file_path)
    if not cnt: 
        return
    
    lines = cnt.splitlines()
    length = len(lines)
    i = 0
    while (i < length):
        if lines[i].startswith("///") or lines[i].startswith("///"):
            k = i
            i += 1
            while (i < length) and (lines[i].startswith('//')):
                i += 1

            if (i < length):
                r = REG_CS_FUNCTION_NAME.search(lines[i])
                if r and r.group(1):
                    doc_item = {'name': r.group(1)}
                    
                    cmnt = parse_comment(lines[k:i])
                    if cmnt: 
                        doc_item.update(cmnt)
                    result.append(doc_item)
                else:
                    i += 1
        else:
            i += 1
            
    return result
    
    
    
#########################
    
def parse_comment(comment_lines):
    """
    Parse comment to extract documentation
    /**
     * function description
     * @name my_function_name //name of function. If not defined uses system name (defined in CPP as COMMAND(function_name... or in CS as function_name = [...)
     * @groups group1 group2  //if not defined uses "system" group
     * @arg1 argument 1 description //not required
     * @arg2 argument 2 description //not required
     * @return what this returns //not required
     * @example example of usage  //not required
     */
     
     or the same with //  :
     
     // function description
     // @groups group1 group2
     // .....
    """
    
    res = {}    
    for line in comment_lines:
        line = line.strip()
        line = re.sub('^\/\/+\s*', "", line)
        line = re.sub("^\*\/\s*", "", line)
        line = re.sub("^\*+\s*", "", line)
        line = re.sub("^\/\*+\s*", "", line)
        if not line:
            continue
        
        r = re.search('^@name ?(.*?)$', line, re.I)
        if r:
            res['name'] = r.group(1)
            continue
    
        r = re.search('^@groups? ?(.*?)$', line, re.I)
        if r:
            groups = r.group(1)
            res['groups'] = [x.strip() for x in re.split(', ', groups)]
            continue
    
        r = re.search('^@example ?(.*?)$', line, re.I)
        if r:
            res['example'] = r.group(1)
            continue
    
        r = re.search('^@return ?(.*?)$', line, re.I)
        if r:
            res['return'] = r.group(1)
            continue
    
        r = re.search('^@arg(\d+) ?(.*?)$', line, re.I)
        if r:
            if not res.get('args'):
                res['args'] = {}
            res['args'][r.group(1)] = {'description': r.group(2)}
            continue      
        
        if not res.get('comments'):
            res['comments'] = line
        else:
            res['comments'] += "\n"+line
        
    return res


def load_func_doc(path):
    """Parse cpp and cs files and return list of documentation dicts"""
    files = do_search(path)
    doc_list = []
    for file in files:
        for ext in CPP_EXTENSIONS:
            if file.endswith(ext):
                d = parse_cpp_file(file)
                if d:
                    doc_list.extend(d)
                break
        for ext in CS_EXTENSIONS:
            if file.endswith(ext):
                d = parse_cs_file(file)
                if d:
                    doc_list.extend(d)
                break
    return doc_list
            

if __name__ == "__main__":
    doc_list = load_func_doc('../../')
    print(doc_list)
