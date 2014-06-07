#include <iostream>
#include <fstream>
#include <string>

#include <spawner.h>
#include <functional>

#include "arguments.h"

//#include <tclap\CmdLine.h>
//
//
//
//class lambda_visitor: public TCLAP::Visitor {
//public:
//    std::function<void()> lambda;
////    default_visitor(){}
//    virtual void visit() {
//        lambda();
//    }
//};
//
//template<typename T>// typename K>
//class another_visitor: public TCLAP::Visitor {
//public:
////    default_visitor(){}
//    another_visitor(T& val): value(val){}
//    T &value;
//    //K *arg;
//    virtual void visit() {
//        //value = arg->value();
//    }
//};
//std::string tmp;
//
//template<unit_t u, degrees_enum d>
//struct restriction_value_s {
//    restriction_value_s(const std::string &s) {} 
//    typedef TCLAP::StringLike ValueCategory;
//    unsigned long value;
//    restriction_value_s& operator=(const std::string &str) {
//        value = convert(value_t(u, d), str, restriction_no_limit);
//        return *this;
//    }
//    operator restriction_t() const {
//        return value;
//    }
//
//};
//
//class program_argument_t: public TCLAP::UnlabeledMultiArg<std::string> {
//};
//
//struct bool_value_s {
//    bool_value_s(const std::string &s) {} 
//    typedef TCLAP::StringLike ValueCategory;
//    bool value;
//    bool_value_s& operator=(const std::string &str) {
//        value = str=="1";//convert(value_t(u, d), str, restriction_no_limit);
//        return *this;
//    }
//    operator restriction_t() const {
//        return value?restriction_limited:restriction_no_limit;
//    }
//
//};
//#define ARGUMENT_VALUE(SHORT_ARG, LONG_ARG, DESC, REQ, DEFAULT, TYPE_VAL, VALUE, ...) do {\
//    auto *visitor = new lambda_visitor();\
//    auto *test_arg = new TCLAP::ValueArg<TYPE_VAL>(SHORT_ARG, LONG_ARG, DESC, REQ, DEFAULT, DESC, cmd, visitor);\
//    visitor->lambda = [test_arg, this, __VA_ARGS__]() {\
//        try {\
//            /**/VALUE = test_arg->getValue();/**/\
//        } catch (...) {\
//            /*exception in smth*/\
//        }\
//    };\
//} while (0)
//
//#define ARGUMENT_VALUE1(SHORT_ARG, LONG_ARG, DESC, REQ, DEFAULT, VALUE) do {\
//    auto *visitor = new another_visitor(VALUE);\
//    auto *test_arg = new TCLAP::ValueArg<std::string>(SHORT_ARG, LONG_ARG, DESC, REQ, DEFAULT, DESC, cmd, visitor);\
//} while (0)
//
//typedef restriction_value_s<unit_time_second, degree_milli> milli_second;
//typedef restriction_value_s<unit_memory_byte, degree_default> byte_value;

#define NEW_CONSOLE_PARSER(PARSER) console_argument_parser_c *console_parser_##PARSER = new console_argument_parser_c
#define NEW_ENVIRONMENT_PARSER(PARSER) environment_variable_parser_c *environment_parser_##PARSER = new environment_variable_parser_c
#define MILLISECOND_CONVERT convert<unit_time_second, degree_milli>
#define BYTE_CONVERT convert<unit_memory_byte, degree_default>
#define BOOL_CONVERT convert_bool
#define STRING_CONVERT
#define ADD_CONSOLE_ARGUMENT(PARSER, ARGUMENTS, VALUE, TYPE_CONVERTER, ...) (console_parser_##PARSER->\
    add_parameter((std::vector<std::string>)ARGUMENTS, [this](std::string &s) -> bool {VALUE=TYPE_CONVERTER(s); __VA_ARGS__ ; return 1;}))
#define ADD_ENVIRONMENT_ARGUMENT(PARSER, ARGUMENTS, VALUE, TYPE_CONVERTER, ...) (environment_parser_##PARSER->\
    add_parameter((std::vector<std::string>)ARGUMENTS, [this](std::string &s) -> bool {VALUE=TYPE_CONVERTER(s); __VA_ARGS__ ; return 1;}))

#define ADD_CONSOLE_ENVIRONMENT_ARGUMENT(PARSER, CONSOLE_ARGUMENTS, ENVIRONMENT_ARGUMENTS, VALUE, TYPE_CONVERTER, ...) do {\
    ADD_CONSOLE_ARGUMENT(PARSER, CONSOLE_ARGUMENTS, VALUE, TYPE_CONVERTER, __VA_ARGS__);\
    ADD_ENVIRONMENT_ARGUMENT(PARSER, ENVIRONMENT_ARGUMENTS, VALUE, TYPE_CONVERTER, __VA_ARGS__);\
    } while (0)


class spawner_base_c {
public:
    spawner_base_c(){}
    virtual void run(int argc, char *argv[]){}
    virtual std::string help() {
        return "Usage:\n\t--legacy=<sp99|sp00|pcms2>\n";
    }
};


class command_handler_c {
protected:
    settings_parser_c parser;
    void add_default_parser();
public:
    bool show_help;
    spawner_base_c *spawner;
    command_handler_c();
    void reset();
    bool parse(int argc, char *argv[]);
    spawner_base_c *create_spawner(const std::string &s);
    void add_parser(abstract_parser_c *p);
};






template<unit_t u, degrees_enum d>
restriction_t convert(const std::string &str) {
    return convert(value_t(u, d), str, restriction_no_limit);
}

restriction_t convert_bool(const std::string &str) {
    return str=="1"?restriction_limited:restriction_no_limit;
}

class spawner_old_c: public spawner_base_c {
    restrictions_class restrictions;
    options_class options;
    bool hide_report;
    bool hide_output;
    bool runas;
    settings_parser_c &parser;
    std::string report_file;
    std::string output_file;
    std::string input_file;
    //std::string program;

    output_buffer_class *create_output_buffer(const std::string &name, const pipes_t &pipe_type, const size_t buffer_size) {
	    output_buffer_class *output_buffer = NULL;
	    if (name == "std") {
            unsigned int color = FOREGROUND_BLUE | FOREGROUND_GREEN;
            if (pipe_type == STD_ERROR_PIPE) {
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
            }
		    output_buffer = new output_stdout_buffer_class(4096, color);
        } else if (name[0] == '*') {
	    } else if (name.length()) {
		    output_buffer = new output_file_buffer_class(name, 4096);
	    }
        return output_buffer;
    }
    input_buffer_class *create_input_buffer(const std::string &name, const size_t buffer_size) {
        input_buffer_class *input_buffer = NULL;
        if (name == "std") {
            input_buffer = new input_stdin_buffer_class(4096);
        } else if (name[0] == '*') {
        } else if (name.length()) {
            input_buffer = new input_file_buffer_class(name, 4096);
        }
        return input_buffer;
    }
public:
    spawner_old_c(settings_parser_c &parser): parser(parser), spawner_base_c(), options(session_class::base_session), hide_report(false), hide_output(false), runas(false) {
        init_arguments();
    }
    virtual void run(int argc, char *argv[]) {

    }
    virtual std::string help() {
        return spawner_base_c::help() + "\
Spawner oprions:\n\
  Argument            Environment variable     Description\n\
    -ml:[n]             SP_MEMORY_LIMIT     ������������ ����� ����������� ������, ���������� �������� (� Mb).\n\
    -tl:[n]             SP_TIME_LIMIT       ������������ ����� ���������� �������� � ���������������� ������ (� ���).\n\
    -d:[n]              SP_DEADLINE         ����� ����������� �������, ����������� �������� (� ���).\n\
    -wl:[n]             SP_WRITE_LIMIT      ������������ ����� ������, ������� ����� ���� ������� ��������� (� Mb).\n\
    -u:[user@domain]    SP_USER             ��� ������������ � �������: User[@Domain]\n\
    -p:[password]       SP_PASSWORD         ������.\n\
    -runas:[0|1]        SP_RUNAS            ������������ ������ RunAs ��� ������� ��������.\n\
    -s:[n]              SP_SECURITY_LEVEL   ������� ������������. ����� ��������� �������� 0 ��� 1.\n\
    -hr:[0|1]           SP_HIDE_REPORT      �� ���������� �����.\n\
    -ho:[0|1]           SP_HIDE_OUTPUT      �� ���������� �������� ����� (STDOUT) ����������.\n\
    -sr:[file]	        SP_REPORT_FILE      ��������� ����� � ����.\n\
    -so:[file]	        SP_OUTPUT_FILE      ��������� �������� ����� � ����.\n\
    -i:[file]           SP_INPUT_FILE       �������� ������� ����� �� �����. \n";
    }
    void init_arguments() {
    
        parser.set_dividers(c_lst(":").vector());

        NEW_CONSOLE_PARSER(old_spawner);
        NEW_ENVIRONMENT_PARSER(old_spawner);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("tl")),   c_lst("SP_TIME_LIMIT"),     this->restrictions[restriction_user_time_limit],  MILLISECOND_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("ml")),   c_lst("SP_MEMORY_LIMIT"),   this->restrictions[restriction_memory_limit],     BYTE_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("s")),    c_lst("SP_SECURITY_LEVEL"), this->restrictions[restriction_security_limit],   BOOL_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("d")),    c_lst("SP_DEADLINE"),       this->restrictions[restriction_processor_time_limit], MILLISECOND_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("wl")),   c_lst("SP_WRITE_LIMIT"),    this->restrictions[restriction_write_limit],      BYTE_CONVERT);


        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("u")),    c_lst("SP_USER"),           this->options.login,    STRING_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("p")),    c_lst("SP_PASSWORD"),       this->options.password, STRING_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("sr")),   c_lst("SP_REPORT_FILE"),    this->report_file,      STRING_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("so")),   c_lst("SP_OUTPUT_FILE"),    this->output_file,      STRING_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("i")),    c_lst("SP_INPUT_FILE"),     this->input_file,       STRING_CONVERT);

        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("runas")), c_lst("SP_RUNAS"),         this->runas,        BOOL_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("ho")),    c_lst("SP_HIDE_OUTPUT"),   this->hide_output,  BOOL_CONVERT);
        ADD_CONSOLE_ENVIRONMENT_ARGUMENT(old_spawner, c_lst(short_arg("hr")),    c_lst("SP_HIDE_REPORT"),   this->hide_report,  BOOL_CONVERT);

        parser.add_parser(console_parser_old_spawner);
        parser.add_parser(environment_parser_old_spawner);
    }
};

typedef std::function<void(std::string)> callback_t;

template<typename T, typename T1>
callback_t param_binder() {
    //return
}

void command_handler_c::add_default_parser() {
    NEW_CONSOLE_PARSER(default_parser);
    NEW_ENVIRONMENT_PARSER(default_parser);
    command_handler_c *self = this;

    /*console_parser_default_parser->add_parameter((std::vector<std::string>)c_lst(long_arg("legacy")), 
        [self](std::string &s) -> bool {
            bool tmp = self->create_spawner(s);
            if (!tmp) return 0 ; 
            return 1;
    });*/
    ADD_CONSOLE_ENVIRONMENT_ARGUMENT(default_parser, c_lst(long_arg("legacy")), c_lst("SP_LEGACY"), bool tmp, this->create_spawner, if (!tmp) return 0);
    ADD_CONSOLE_ARGUMENT(default_parser, c_lst(short_arg("h"), long_arg("help")), show_help, 1; , parser.stop());
    parser.add_parser(console_parser_default_parser);
    parser.add_parser(environment_parser_default_parser);
}
command_handler_c::command_handler_c(): spawner(NULL) {
//    reset();
    create_spawner("sp00");
}
void command_handler_c::reset() {
    if (spawner) {
        delete spawner;
        spawner = NULL;
    }
    parser.clear_parsers();
    add_default_parser();
    parser.set_dividers(c_lst("=").vector());
}
spawner_base_c *command_handler_c::create_spawner(const std::string &s) {
    reset();
    if (s == "sp99") {
        spawner = new spawner_old_c(this->parser);
    } else if (s == "sp00") {
        spawner = new spawner_base_c();
        //new spawner_old_c(this->parser);
    }
    return spawner;
}
void command_handler_c::add_parser(abstract_parser_c *p) {
    parser.add_parser(p);
    p->invoke_initialization(parser);
}

bool command_handler_c::parse(int argc, char *argv[]) {
//    reset();
    parser.parse(argc, argv);
    if (show_help && spawner) {
        std::cout << spawner->help();
    }
    return true;
}


int main(int argc, char *argv[]) {
    //spawner_base_c *spawner = new spawner_old_c();
    //spawner->run(argc, argv);
    command_handler_c handler;
    handler.parse(argc, argv);




    return 0;//*/
    /*
    spawner_c spawner(argc, argv);


    spawner.init();
    spawner.run();
    
	return 0;//*/
}
