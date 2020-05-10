char* imports =
    "from copy import deepcopy\n"                                   \
    "from copy import copy\n"                                       \
    "\n"                                                            \
    "\n"                                                            \
    ;

char* slice_class_name = "__codegen_helper_slice";

char* slice_class = \
"class __codegen_helper_slice:                          \n"
"    def __init__(self, slice_object=None):             \n"
"        if slice_object is not None:                   \n"
"            self.len = slice_object.len                \n"
"            self.cap = slice_object.cap                \n"
"            self.arr = slice_object.arr                \n"
"            self.duped = slice_object.duped            \n"
"            slice_object.duped[0] += 1                 \n"
"        else:                                          \n"
"            self.len = 0                               \n"
"            self.cap = 0                               \n"
"            self.duped = [1]                           \n"
"            self.arr = []                              \n"
"                                                       \n"
"    @classmethod                                       \n"
"    def dup(cls, rhs):                                 \n"
"        new_slice = cls()                              \n"
"        new_slice.len = rhs.len                        \n"
"        new_slice.cap = rhs.cap                        \n"
"        new_slice.arr = rhs.arr                        \n"
"        rhs.duped[0] += 1                              \n"
"        new_slice.duped = rhs.duped                    \n"
"        return new_slice                               \n"
"                                                       \n"
"    def append(self, element):                         \n"
"        copy = self.dup(self)                          \n"
"                                                       \n"
"        if copy.cap == 0:                              \n"
"            copy.cap = 2                               \n"
"                                                       \n"
"        if copy.len == copy.cap:                       \n"
"            if copy.duped[0] != 1:                     \n"
"               temp = []                               \n"
"               for x in copy.arr:                      \n"
"                   temp.append(x)                      \n"
"               copy.arr = temp                         \n"
"               copy.duped = [1]                        \n"
"            copy.cap = copy.cap * 2                    \n"
"        if copy.len >= len(copy.arr):                  \n"
"            copy.arr.append(type(element)(element))  \n"
"        else:                                          \n"
"            copy.arr[copy.len] = type(element)(element)\n"
"                                                       \n"
"        copy.len = copy.len + 1                        \n"
"        return copy                                    \n"
"                                                       \n"
"    def fast_append(self, element):                    \n"
"        if self.cap == 0:                              \n"
"            self.cap = 2                               \n"
"                                                       \n"
"        if self.len == self.cap:                       \n"
"            if self.duped[0] != 1:                     \n"
"               temp = []                               \n"
"               for x in self.arr:                      \n"
"                   temp.append(x)                      \n"
"               self.arr = temp                         \n"
"               self.duped = [1]                        \n"
"            self.cap = self.cap * 2                    \n"
"        if self.len >= len(self.arr):                  \n"
"            self.arr.append(type(element)(element))  \n"
"        else:                                          \n"
"            self.arr[self.len] = type(element)(element)\n"
"                                                       \n"
"        self.len = self.len + 1                        \n"
"                                                       \n"
"    def __getitem__(self, index):                      \n"
"        if (index >= self.len):                        \n"
"            raise IndexError(                          \n"
"                'Slice index out of range.'            \n"
"            )                                          \n"
"        return self.arr[index]                         \n"
"                                                       \n"
"    def __setitem__(self, index, val):                 \n"
"        if (index >= self.len):                        \n"
"            raise IndexError(                          \n"
"                'Slice index out of range.'            \n"
"            )                                          \n"
"        self.arr[index] = val                          \n"
"    def __del__(self):                                 \n"
"        self.duped[0] -=1                              \n"
"                                                       \n"
"                                                   \n\n\n";

char* fill_list_function_name = "__codegen_helper_fill_list";

char* fill_list_function =
    "def __codegen_helper_fill_list(val, size):\n"                  \
    "   ret = []\n"                                                 \
    "   for _ in range(size):\n"                                    \
    "       ret.append(deepcopy(val))\n"                            \
    "   return ret\n"                                               \
    "\n"                                                            \
    "\n"                                                            \
    ;

char* _get_bool_string_function =                                   \
    "def __codegen_helper_get_bool_str(val):\n"                     \
    "   if val:\n"                                                  \
    "       return 'true'\n"                                        \
    "   else:\n"                                                    \
    "       return 'false'\n"                                       \
    "\n"                                                            \
    "\n"                                                            \
    ;
