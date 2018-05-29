#########################################################################
# (c) Copyright, Real-Time Innovations, All rights reserved.
#
# Permission to modify and use for internal purposes granted.
# This software is provided "as is", without warranty, express or implied.
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Converts any text file into a C file with a definition of
# a string array variable containing the text file in several
# fixed-length strings.
#
# Useful when there is the need to parse an XML string rather than the file
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Example:
#
#   perl cStringifyFile.pl MyText.txt MY_TEXT_STR
#
#   Being MyText.txt like this:
#
#       Hello World
#       This is a "test" file
#
#   Would generate this C file, assuming that the string length is 10:
#
#       #define MY_TEXT_STR_SIZE_DEF 4
#       const int MY_TEXT_STR_SIZE = MY_TEXT_STR_SIZE_DEF;
#       const char * MY_TEXT_STR[MY_TEXT_STR_SIZE_DEF] = {
#       "Hello Worl",
#       "d\nThis is ",
#       "a \"test\" f",
#       "ile" };
#########################################################################


sub stringifyFile {

    local($CHARS_PER_STR) = 1024;
    local($cur_cnt) = 0;
    local($tot_cnt) = 0;
    local($str_cnt) = 0;
    local($output) = "";
    local($file) = "";
    local($var_name) = "MY_STRINGS";
    local($size_var_name) = "";
    local($size_def_name) = "";
    local($total_size_var_name) = "";
    local($main_code) = "";
    local($modifier) = "";

    if( (scalar @_) > 0)
    {
            $file = $_[0];
        if( (scalar @_) > 1)
            {
                $var_name = $_[1];

                    if( (scalar @_) > 2)
                    {
                    $modifier = $_[2];
                }
            }
    }
    else
    {
        die "Format: <file> [<array_var_name>] [<size_var_name>] [<modifier>]\n";
    }

    $size_var_name = $var_name . "_SIZE";
    $size_def_name = $size_var_name . "_DEF";
    $total_size_var_name = $var_name . "_TOTAL_SIZE";
    open FILE, "<$file" or die "$file cannot be read";

    while(<FILE>)
    {
        @cs = split //;
            foreach $c (@cs)
            {
                if($c eq "\n")
                {
                    $actual_c = "\\n";
                }
                elsif($c eq "\"")
                {
                    $actual_c = "\\\"";
                }
                elsif($c eq "\\")
                {
                    $actual_c = "\\\\";
                }
                else
                {
                    $actual_c = $c;
                }

                $output = $output . $actual_c;

                    $cur_cnt = $cur_cnt + 1;
                $tot_cnt = $tot_cnt + 1;

                if($cur_cnt == $CHARS_PER_STR)
                {
                    $cur_cnt = 0;
                    $str_cnt = $str_cnt + 1;

                    $output = $output . "\",\n\"";
                }
            }
    }

    $tot_cnt  = $tot_cnt + 3;

    if($cur_cnt > 0)
    {
        $output = $output . "\" };\n\n";
    }
    else
    {
        $output = $output . "\"\" };\n\n";
    }
    $str_cnt = $str_cnt + 1;


    if($modifier eq "")
    {
        $output = "#include <string.h>\n" .
                "#define $size_var_name $str_cnt\n" .
                "#define $total_size_var_name $tot_cnt\n" .
                "const char * " . $var_name . "[" . $size_var_name . "]" .
                " = {\n\"" . $output;
    }
    else
    {
        $output = "#include <string.h>\n" .
                "#include \"log/log_makeheader.h\"\n" .
                "#pragma begin_$modifier\n" .
                "#define $size_var_name $str_cnt\n" .
                "#define $total_size_var_name $tot_cnt\n" .
                "#pragma end_$modifier\n" .
                "$modifier const char * " . $var_name .
                "[" . $size_var_name . "]" .
                " = {\n\"" . $output;
    }


    if(!($modifier eq ""))
    {
        $output = $output . "#pragma begin_$modifier\n";
    }
    $output = $output .
            "#define $var_name" . "_asString(str) {\\\n" .
            "       int i;\\\n" .
            "       (str)[0] = 0;\\\n" .
            "       for(i = 0; i < $size_var_name; ++i) {\\\n" .
            "            strcat(str, $var_name"."["."i]);\\\n" .
            "       }\\\n" .
            "}\n";

    if(!($modifier eq ""))
    {
        $output = $output . "#pragma end_$modifier\n\n";
    }

    return $output;

}

print stringifyFile @ARGV;
