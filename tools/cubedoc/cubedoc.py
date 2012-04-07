#!/usr/bin/python3
"""
Remod Cubescript documentation generator
@author: oramahmaalhur
@version 0.1
"""

import os
import os.path
import re

#constants
SYSTEM_DEFAULT_GROUP = "system"
CUSTOM_DEFAULT_GROUP = "server"

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
        rs = {}
        if comments:
            rs = {'name': name, 'path': file_path, 'group': CUSTOM_DEFAULT_GROUP}
            cmnts = parse_comment(comments.split('\n')) 
            rs.update(cmnts)
        else:
            #if no comment it's system group
            rs = {'name': name, 'path': file_path, 'group': SYSTEM_DEFAULT_GROUP}         
            
        if not rs.get('args'):
            rs['args'] = {}
            
        for i in range(len(arg_types)):
            if rs['args'].get(str(i+1)):
                rs['args'][str(i+1)].update({'type': TYPE_NAMES.get(arg_types[i])})
            else:
                rs['args'][str(i+1)] = {'type': TYPE_NAMES.get(arg_types[i])}
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
                    doc_item = {'name': r.group(1), 'group': CUSTOM_DEFAULT_GROUP, 'path': file_path}
                    
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
    
        r = re.search('^@group? ?(.*?)$', line, re.I)
        if r:
            res['group'] = r.group(1)
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
            res['args'][r.group(1)] = {'descr': r.group(2)}
            continue      
        
        if not res.get('descr'):
            res['descr'] = line
        else:
            res['descr'] += "\n"+line
        
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
            
            
            
def parse_template(template, data):
    "Simple template parser"
    class Node:
        ROOT="ROOT"
        TEXT="TEXT"
        LOOP="LOOP"
        IF="IF"
        TAGS = {"loop": LOOP, "if": IF}
        PARSE_EXPRESSION = re.compile('\$\{([^\}]+)\}')
        PARSE_VARIABLE = re.compile('var="([^"]+)"')
        PARSE_VALUE = re.compile('value="\$\{([^\}]+)\}"')
        
        def __init__(self, parent, type, value):
            self.parent = parent
            self.type = type
            self.value = value
            self.children = []
            
        def add(self, type, value):
            child = Node(self, type, value)
            self.children.append(child)
            return child
        
        def __str__(self):
            s = '[Node:'+self.type+'\n'
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
            
            
            if self.type == Node.TEXT:
                s = self.PARSE_EXPRESSION.sub(lambda x: eval(x.group(1), data), self.value)
                
            elif self.type == Node.LOOP:
                var = self.PARSE_VARIABLE.search(self.value).group(1)
                val = self.PARSE_VALUE.search(self.value).group(1)
                loop_items = eval(val, data)
                for l in loop_items:
                    d = data
                    d[var] = l
                    s += self._render_children(d)
                    
            elif self.type == Node.IF:
                val = self.PARSE_VALUE.search(self.value).group(1)
                condition = eval(val, data)
                if condition:
                    s += self._render_children(data)
            else:
                s += self._render_children(data)
            return s
        
    result = ""
    
    #parse template to find loop and condition tags
    r = re.compile("<(\/?)py:(\w+)\s?([^>]*)>", re.I)
    root = Node(None, Node.ROOT, "")
    
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
        node.add(Node.TEXT, text)
        
        if is_closing:
            node = node.parent
        else:
            node = node.add(Node.TAGS[tag], expression)
        
        start = end
    
    node.add(Node.TEXT, template[end:])
    
    return root.render(data)
            

if __name__ == "__main__":
    template_path = "template.html"
    help_file_path = "tutorial.html"
    cs_path = "../../"
    
    t = open(template_path)
    template_content = t.read()
    t.close()
    
    
    ## resorting functions by groups
    
    data = {'functions': {}, 'groups': []}
    
    for f in load_func_doc(cs_path):
        group = f['group']
        if not data['groups'].count(group):
            data['groups'].append(group)
            data['functions'][group] = []
        if 'descr' not in f:
            f['descr'] = ""
        data['functions'][group].append(f)

    for group in data['groups']:
        data['functions'][group].sort(key=lambda k: k['name'])
        
    help_file_content = parse_template(template_content, data)
    
    t = open(help_file_path, "w+")
    t.write(help_file_content)
    t.close()
    
