/* a small testing framework, inspired by greatest.h */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#define DETEST_MSG_MAX_SIZE 1024

#define DETEST_COLOR_GREEN "\033[1;32m"
#define DETEST_COLOR_YELLOW "\033[1;33m"
#define DETEST_COLOR_RED "\033[1;31m"
#define DETEST_COLOR_RESET "\033[1;0m"

#define DETEST_PRINTF_IF_ENABLED(_fmt,...) do { if(!detest_all_suites.mute) fprintf(stderr,_fmt, ## __VA_ARGS__); } while(0)
#define DETEST_PRINTF_ALWAYS(_fmt,...) do { fprintf(stderr,_fmt, ## __VA_ARGS__); } while(0)
#define DETEST_ERROR(_fmt,...) do { fprintf(stderr,_fmt, ## __VA_ARGS__); } while(0)

#define DETEST_CTRF_KEY_VALUE_LAST(_key,_value) do {\
  fprintf(f,"\"%s\": \n",_key);\
  detest_fputs_escaped(_value, f);\
} while(0)
#define DETEST_CTRF_KEY_VALUE(_key,_value) do { DETEST_CTRF_KEY_VALUE_LAST(_key,_value); fputc(',',f); } while(0)
#define DETEST_CTRF_TEST_NAME(_suite,_test) fprintf(f,"\"name\": \"%s.%s\",\n",_suite,_test)
#define DETEST_CTRF_KEY_VALUE_INT(_key,_value) fprintf(f,"\"%s\": %d,\n",_key,_value)
#define DETEST_CTRF_KEY_VALUE_LINT(_key,_value) fprintf(f,"\"%s\": %ld,\n",_key,_value)
#define DETEST_CTRF_KEY_VALUE_INT_LAST(_key,_value) fprintf(f,"\"%s\": %d\n",_key,_value)
#define DETEST_CTRF_KEY_VALUE_LINT_LAST(_key,_value) fprintf(f,"\"%s\": %ld\n",_key,_value)
#define DETEST_CTRF_OPEN_OBJECT(_name) fprintf(f,"\"%s\": {\n",_name)
#define DETEST_CTRF_CLOSE_OBJECT fprintf(f,"},\n")
#define DETEST_CTRF_CLOSE_OBJECT_LAST fprintf(f,"}\n")
#define DETEST_CTRF_OPEN_ARRAY(_name) fprintf(f,"\"%s\": [\n",_name)
#define DETEST_CTRF_CLOSE_ARRAY fprintf(f,"],\n")
#define DETEST_CTRF_CLOSE_ARRAY_LAST fprintf(f,"]\n")

#define DETEST_FAIL(_fmt,...) do {\
  (test)->failure_file = __FILE__;\
  (test)->failure_line = __LINE__;\
  (test)->failed = 1;\
  snprintf((test)->failure_message, sizeof((test)->failure_message), _fmt, ## __VA_ARGS__);\
  (test)->next_failed = detest_all_suites.failed_list;\
  detest_all_suites.failed_list = test;\
  detest_all_suites.n_failed_tests++;\
  return;\
} while(0)

#define DETEST_TEST(_suite,_test) \
  void detest_test_ ## _suite ## _ ## _test(detest_test_t* test);\
  __attribute__ ((constructor))  \
  void detest_test_ ## _suite ## _ ## _test ## _register() { \
    detest_register_test(#_suite,#_test,__FILE__,__LINE__, detest_test_ ## _suite ## _ ## _test);\
  }\
  void detest_test_ ## _suite ## _ ## _test(detest_test_t* test) 


typedef struct detest_suite_s detest_suite_t;
typedef struct detest_test_s detest_test_t;
typedef void (*detest_func_t)(detest_test_t* test);

void detest_register_test(const char* suite_name,
      const char* name, const char* def_file, 
      unsigned def_line, detest_func_t body);

struct detest_test_s {
    const char*     name;
    detest_suite_t* suite;
    const char*     definition_file;
    unsigned        definition_line;
    detest_func_t   body;

detest_test_t*  next_failed;
    unsigned    n_executions;
    //unsigned    n_asserts;
    unsigned    failed;
    const char* failure_file;
    unsigned    failure_line;
    char        failure_message[DETEST_MSG_MAX_SIZE];
    unsigned    total_assertions;
    unsigned    duration;
};
  
struct detest_suite_s {
    const char* name;
    detest_test_t* tests;
    unsigned    n_tests;
    unsigned allocated_tests;
};

typedef struct {\
    detest_suite_t* suites;\
    unsigned n_suites;
    unsigned allocated_suites;
    unsigned total_assertions;
    unsigned n_failed_tests;
    unsigned n_skipped_tests;
    unsigned n_run_tests;
    detest_test_t* failed_list;

    time_t start_time;
    time_t stop_time;

    const char* suite_filter;
    const char* test_filter;

    const char* ctrf_filename;

    bool mute;
} detest_status_t;

extern detest_status_t detest_all_suites;\

/////// definition that should in a macro "DETEST_DEFINITIONS"
#define DETEST_DEFINITIONS \
  \
  detest_status_t detest_all_suites = {};\
  \
  static int detest_name_match(const char *name, const char *filter) {\
    size_t offset = 0;\
    size_t filter_len = strlen(filter);\
    while (name[offset] != '\0') {\
        if (name[offset] == filter[0]) {\
            if (0 == strncmp(&name[offset], filter, filter_len)) {\
                return 1;\
            }\
        }\
        offset++;\
    }\
    \
    return 0;\
  }\
  \
  detest_suite_t* detest_find_suite(const char* name) {\
    for(unsigned i=0; i<detest_all_suites.n_suites; i++) \
      if(strcmp(detest_all_suites.suites[i].name, name) == 0) return &detest_all_suites.suites[i];\
    return 0;\
  }\
  void detest_print_usage(const char* name) {\
    DETEST_PRINTF_ALWAYS("Usage:\n");\
    DETEST_PRINTF_ALWAYS("%s <options>\n",name);\
    DETEST_PRINTF_ALWAYS("\nOptions\n\n");\
    DETEST_PRINTF_ALWAYS("\t-s <match>\n\t\t suite filter\n");\
    DETEST_PRINTF_ALWAYS("\t-t <match>\n\t\t test filter\n");\
    DETEST_PRINTF_ALWAYS("\t-j <filename>\n\t\t save test report in CRTF json format (https://www.ctrf.io/)\n");\
    DETEST_PRINTF_ALWAYS("\t-h\n\t\t print this help message and exit\n");\
    DETEST_PRINTF_ALWAYS("\t-q\n\t\t run quietly\n");\
    DETEST_PRINTF_ALWAYS("\t-l\n\t\t list of tests and exit\n");\
  }\
  \
  void detest_list_suites();\
  static void detest_parse_options(int ac, char **av) {\
    detest_all_suites.mute = false;\
    for (int i = 1; i < ac; i++) {\
        if (av[i][0] == '-') {\
            char f = av[i][1];\
            if ((f == 's' || f == 't' || f == 'j') && ac <= i + 1) {\
                /*detest_usage(argv[0]);*/ exit(EXIT_FAILURE);\
            }\
            switch (f) {\
            case 'h':\
                detest_print_usage(av[0]); exit(0); break;\
            case 'j':\
                detest_all_suites.ctrf_filename = strdup(av[i+1]); break;\
            case 'l':\
                detest_list_suites(); exit(0); break;\
            case 'q': /* suite name filter */\
                detest_all_suites.mute = true; break;\
            case 's': /* suite name filter */\
                detest_all_suites.suite_filter = strdup(av[i + 1]); i++; break;\
            case 't': /* test name filter */\
                detest_all_suites.test_filter = strdup(av[i + 1]); i++; break;\
            }\
        }\
    }\
  }\
  detest_suite_t* detest_find_or_add_suite(const char* name) {\
    detest_suite_t* suite = detest_find_suite(name); \
    if(suite) return suite;\
    \
    assert(detest_all_suites.n_suites < detest_all_suites.allocated_suites);\
    unsigned index  = detest_all_suites.n_suites++;\
    suite = &detest_all_suites.suites[index];\
    suite->name = name;\
    suite->allocated_tests = 100;\
    suite->n_tests = 0;\
    suite->tests = calloc(suite->allocated_tests,sizeof(suite->tests[0]));\
    return suite;\
  }\
  static void detest_fputs_escaped(const char *s, FILE *f) {\
   if (!s || !f) return;\
    fputc('"',f);\
    for (; *s; s++) {\
      switch (*s) {\
        case '"':\
          fputc('\\', f);\
          fputc('"', f);\
          break;\
        case '\\':\
          fputc('\\', f);\
          fputc('\\', f);\
          break;\
        default:\
          fputc(*s, f);\
          break;\
      }\
    }\
    fputc('"',f);\
  }\
  \
  time_t detest_epoch_millisec() {\
    struct timeval  tv;\
    gettimeofday(&tv, NULL);\
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;\
  }\
  \
  void detest_init_test(detest_test_t* test, detest_suite_t* suite, \
      const char* name, const char* def_file, \
      unsigned def_line, detest_func_t body) {\
    memset(test,0,sizeof(detest_test_t));\
    test->name = name;\
    test->definition_file = def_file;\
    test->definition_line = def_line;\
    test->suite = suite;\
    test->body = body;\
  }\
  detest_test_t* detest_add_test(detest_suite_t* suite, const char* name,\
      const char* def_file, unsigned def_line, detest_func_t body) {\
    for(unsigned i=0; i<suite->n_tests; i++) \
      assert(strcmp(suite->tests[i].name, name) != 0);\
    assert(suite->n_tests < suite->allocated_tests);\
    detest_test_t* test = &suite->tests[suite->n_tests++];\
    assert(test);\
    detest_init_test(test, suite, name, def_file, def_line, body);\
    return test;\
  };\
  \
  int detest_run_test(detest_test_t* test) {\
    if(detest_all_suites.test_filter && !detest_name_match(test->name,detest_all_suites.test_filter))\
      return 0;\
    detest_all_suites.n_run_tests++;\
    DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_GREEN "[ RUN      ]" DETEST_COLOR_RESET  " %s.%s\n",test->suite->name,test->name);\
    unsigned start = detest_epoch_millisec();\
    test->body(test);\
    test->duration = detest_epoch_millisec() - start;\
    if(test->failed) {\
      DETEST_PRINTF_IF_ENABLED("At %s:%d - ",test->failure_file,test->failure_line);\
      DETEST_PRINTF_IF_ENABLED("%s\n",test->failure_message);\
      DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_GREEN "[     " DETEST_COLOR_RED "FAIL" DETEST_COLOR_GREEN " ]" DETEST_COLOR_RESET " %s.%s\n",test->suite->name,test->name);\
    } else {\
      if(test->total_assertions) \
        DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_GREEN "[       OK ]" DETEST_COLOR_RESET " %s.%s (%d msec, %d assertions)\n",test->suite->name,test->name,test->duration,test->total_assertions);\
      else {\
        DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_YELLOW "Warning: no assertions" DETEST_COLOR_RESET "\n");\
        DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_YELLOW "[  WARNING ]" DETEST_COLOR_RESET " %s.%s (%d msec, %d assertions)\n",test->suite->name,test->name,test->duration,test->total_assertions);\
      };\
    }\
    return 1;\
  }\
  static void detest_catch_sigv() {\
    DETEST_PRINTF_IF_ENABLED(DETEST_COLOR_RED "ERROR: Segment violation fault" DETEST_COLOR_RESET ", exiting\n");\
    exit(1);\
  }\
  void detest_init_all_suites() {\
    if(detest_all_suites.allocated_suites == 0) {\
      memset(&detest_all_suites,0,sizeof(detest_all_suites));\
      detest_all_suites.n_suites = 0;\
      detest_all_suites.allocated_suites = 100;\
      detest_all_suites.suites = calloc(100,sizeof(detest_suite_t));\
      signal(SIGSEGV, detest_catch_sigv); \
    }\
  }\
  void detest_register_test(const char* suite_name,\
      const char* name, const char* def_file,\
      unsigned def_line, detest_func_t body) {\
    detest_init_all_suites();\
    detest_suite_t* suite = detest_find_or_add_suite(suite_name);\
    assert(suite);\
    detest_add_test(suite,name,def_file,def_line,body);\
  }\
  void detest_list_suite_tests(detest_suite_t* suite) {\
    DETEST_PRINTF_IF_ENABLED("suite: '%s'\n",suite->name);\
    for(unsigned i=0; i<suite->n_tests; i++) \
      DETEST_PRINTF_IF_ENABLED("   - test: '%s'\n",suite->tests[i].name);\
  }\
  void detest_list_suites() {\
    for(unsigned i=0; i<detest_all_suites.n_suites; i++) \
      detest_list_suite_tests(&detest_all_suites.suites[i]);\
  }\
  void detest_run_suite(detest_suite_t* suite) {\
    if(detest_all_suites.suite_filter && !detest_name_match(suite->name,detest_all_suites.suite_filter))\
      return;\
    DETEST_PRINTF_IF_ENABLED("\n\033[1;32m[----------]\033[1;0m suite %s\n",suite->name);\
    unsigned count = 0;\
    for(unsigned i=0; i<suite->n_tests; i++) \
      count += detest_run_test(&suite->tests[i]);\
    DETEST_PRINTF_IF_ENABLED("\033[1;32m[----------]\033[1;0m %d tests from %s\n",count,suite->name);\
  }\
  void detest_print_final_stats() {\
    DETEST_PRINTF_ALWAYS("\n\nTotal tests run: %d, failed: %d\n",\
      detest_all_suites.n_run_tests,\
      detest_all_suites.n_failed_tests);\
    detest_test_t* test = detest_all_suites.failed_list;\
    if(detest_all_suites.n_failed_tests) {\
      DETEST_PRINTF_ALWAYS("Failed tests:\n");\
      while(test) {\
        DETEST_PRINTF_ALWAYS("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");\
        DETEST_PRINTF_ALWAYS("       Test: %s.%s\n",test->suite->name,test->name);\
        DETEST_PRINTF_ALWAYS("     Reason: %s\n",test->failure_message);\
        DETEST_PRINTF_ALWAYS("    Locatio: %s:%d\n",test->failure_file,test->failure_line);\
        test = test->next_failed;\
      }\
    } else {\
      DETEST_PRINTF_ALWAYS(DETEST_COLOR_GREEN "\n\nAll test succeeded\n\n" DETEST_COLOR_RESET);\
    }\
  }\
  static const char* detest_status_string(detest_test_t* test) {\
    if(test->failed) return "failed";\
    return "passed";\
  }\
  static void detest_save_ctrf_report(const char* filename) {\
    FILE* f = fopen(filename,"w");\
    if(!f) {\
      DETEST_ERROR("Error: cannot open '%s' for writing\n",filename);\
      return;\
    }\
    fprintf(f,"{\n"); /* main object */ \
    DETEST_CTRF_KEY_VALUE("reportFormat","CTRF");\
    DETEST_CTRF_KEY_VALUE("specVersion","0.0.0");\
    DETEST_CTRF_OPEN_OBJECT("results");\
    DETEST_CTRF_OPEN_OBJECT("tool");\
    DETEST_CTRF_KEY_VALUE_LAST("name","detest");\
    DETEST_CTRF_CLOSE_OBJECT;\
    DETEST_CTRF_OPEN_OBJECT("summary");\
    DETEST_CTRF_KEY_VALUE_INT("tests",detest_all_suites.n_run_tests);\
    DETEST_CTRF_KEY_VALUE_INT("passed",detest_all_suites.n_run_tests - detest_all_suites.n_failed_tests);\
    DETEST_CTRF_KEY_VALUE_INT("failed",detest_all_suites.n_failed_tests);\
    DETEST_CTRF_KEY_VALUE_INT("pending",0);\
    DETEST_CTRF_KEY_VALUE_INT("other",0);\
    DETEST_CTRF_KEY_VALUE_LINT("start",detest_all_suites.start_time);\
    DETEST_CTRF_KEY_VALUE_LINT_LAST("stop",detest_all_suites.stop_time);\
    DETEST_CTRF_CLOSE_OBJECT;\
    DETEST_CTRF_OPEN_ARRAY("tests");\
    bool first = true;\
    for(unsigned i=0; i<detest_all_suites.n_suites; i++) {\
      detest_suite_t* suite = &detest_all_suites.suites[i];\
      for(unsigned j=0; j<suite->n_tests; j++) {\
        detest_test_t* test = &suite->tests[j];\
        if(!first) fprintf(f,",\n");\
        first = false;\
        fprintf(f,"{\n");\
        DETEST_CTRF_TEST_NAME(suite->name,test->name);\
        DETEST_CTRF_KEY_VALUE_INT("duration",0);\
        if(test->failed) {\
          DETEST_CTRF_KEY_VALUE("message",test->failure_message);\
          DETEST_CTRF_KEY_VALUE("filePath",test->failure_file);\
          DETEST_CTRF_KEY_VALUE_INT("line",test->failure_line);\
        }\
        DETEST_CTRF_KEY_VALUE_LAST("status",detest_status_string(test));\
        fprintf(f,"}");\
      }\
    }\
    fprintf(f,"\n");\
    DETEST_CTRF_CLOSE_ARRAY_LAST;\
    DETEST_CTRF_CLOSE_OBJECT_LAST;\
    fprintf(f,"}\n");\
    fclose(f);\
  }\
  static int detest_run(int ac, char** av) {\
    detest_parse_options(ac, av);\
    detest_all_suites.start_time = detest_epoch_millisec();\
    if(detest_all_suites.suite_filter) DETEST_PRINTF_IF_ENABLED("Suite filter: %s\n",detest_all_suites.suite_filter);\
    if(detest_all_suites.test_filter) DETEST_PRINTF_IF_ENABLED(" Test filter: %s\n",detest_all_suites.test_filter);\
    for(unsigned i=0; i<detest_all_suites.n_suites; i++) {\
      detest_run_suite(&detest_all_suites.suites[i]);\
    }\
    detest_all_suites.stop_time = detest_epoch_millisec();\
    detest_print_final_stats();\
    if(detest_all_suites.ctrf_filename) detest_save_ctrf_report(detest_all_suites.ctrf_filename);\
    return detest_all_suites.n_failed_tests;\
  }

#define DETEST_COMMON_ASSERT do {\
   detest_all_suites.total_assertions++;\
   test->total_assertions++;\
} while(0)


#define ASSERT(_cond) do {\
  DETEST_COMMON_ASSERT;\
  if(!(_cond)) {\
    DETEST_FAIL("expected '%s' to be true",#_cond);\
  }\
} while(0)

#define ASSERT_FALSE(_cond) do {\
  DETEST_COMMON_ASSERT;\
  if((_cond)) {\
    DETEST_FAIL("expected '%s' to be false",#_cond);\
  }\
} while(0)

#define ASSERT_EQ_INT(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(_aa!=_bb) {\
    DETEST_FAIL("expected '%s' (which is %d) to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_NE_INT(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(_aa==_bb) {\
    DETEST_FAIL("expected '%s' (which is %d) not to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_EQ_STR(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  const char* _aa = _a;\
  const char* _bb = _b;\
  if(strcmp(_aa,_bb)!=0) {\
    DETEST_FAIL("expected %s (which is %s) to be equal to %s (which is %s)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_EQ(_a,_b) ASSERT_EQ_INT(_a,_b)
#define ASSERT_NE(_a,_b) ASSERT_NE_INT(_a,_b)

#define ASSERT_LT(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(!(_aa<_bb)) {\
    DETEST_FAIL("expected '%s' (which is %d) to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_LE(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(!(_aa<=_bb)) {\
    DETEST_FAIL("expected '%s' (which is %d) to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_GE(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(!(_aa>=_bb)) {\
    DETEST_FAIL("expected '%s' (which is %d) to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)

#define ASSERT_GT(_a,_b) do {\
  DETEST_COMMON_ASSERT;\
  int _aa = _a;\
  int _bb = _b;\
  if(!(_aa>_bb)) {\
    DETEST_FAIL("expected '%s' (which is %d) to be equal to '%s' (which is %d)",\
      #_a,_aa,#_b,_bb);\
  }\
} while(0)



