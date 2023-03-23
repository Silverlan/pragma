--- @meta
--- 
--- @class string
string = {}

--- 
--- @param str string
--- @return string ret0
function string.hash(str) end

--- 
--- @param values 
--- @return string ret0
--- @overload fun(values: , joinChar: string): string
function string.join(values) end

--- 
--- @param a string
--- @param b string
--- @return bool ret0
--- @overload fun(arg0: string, arg1: string, arg2: bool): bool
--- @overload fun(a: int, b: int, caseSensitive: bool, len: int): bool
function string.compare(a, b) end

--- 
--- @param i string
--- @param numDigits int
--- @return string ret0
function string.fill_zeroes(i, numDigits) end

--- 
--- @param s string
--- @return string ret0
function string.remove_quotes(s) end

--- 
--- @param subject string
--- @param from string
--- @param to string
--- @return string ret0
function string.replace(subject, from, to) end

--- 
--- @param str string
--- @param delimiter string
--- @return any ret0
function string.split(str, delimiter) end

--- 
--- @param s string
--- @return string ret0
function string.remove_whitespace(s) end

--- 
--- @param str string
--- @return bool ret0
function string.is_number(str) end

--- 
--- @param str string
--- @return bool ret0
function string.is_integer(str) end

--- 
--- @param baseElement string
--- @param inElements 
--- @param limit int
--- @param outSimilarElements any
--- @param outSimilarities any
function string.find_similar_elements(baseElement, inElements, limit, outSimilarElements, outSimilarities) end

--- 
--- @param str string
--- @return string ret0
function string.snake_case_to_camel_case(str) end

--- 
--- @param s0 string
--- @param s1 string
--- @return number ret0
function string.calc_levenshtein_similarity(s0, s1) end

--- 
--- @param str string
--- @return string ret0
function string.camel_case_to_snake_case(str) end

--- 
--- @param s0 string
--- @param s1 string
--- @return int ret0
function string.calc_levenshtein_distance(s0, s1) end

--- 
--- @param s0 string
--- @param s1 string
--- @param outStartIdx int
--- @param outLen int
--- @param outEndIdx int
function string.find_longest_common_substring(s0, s1, outStartIdx, outLen, outEndIdx) end


