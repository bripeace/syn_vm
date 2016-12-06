// Stuff needed to make cmocka work right

#ifdef UNIT_TEST

#ifdef assert
#undef assert
#endif /* assert */
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__)
void mock_assert(const int result, const char* expression, const char *file,
                 const int line);

/* main is defined in the unit test so redefine name of the the main function
 * here. */
#define main example_main

#define static

#endif

