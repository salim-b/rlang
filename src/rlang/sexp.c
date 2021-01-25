#include "rlang.h"

#define PRECIOUS_DICT_INIT_SIZE 256

static
struct r_dict precious_dict = { 0 };

#include "decl/sexp-decl.h"


void r_preserve(sexp* x) {
  KEEP(x);

  sexp* stack = r_dict_get0(&precious_dict, x);
  if (!stack) {
    stack = KEEP(new_precious_stack(x));
    r_dict_put(&precious_dict, x, stack);
    FREE(1);
  }

  push_precious(stack);
  FREE(1);
}

void r_unpreserve(sexp* x) {
  sexp* stack = r_dict_get0(&precious_dict, x);
  if (!stack) {
    r_abort("Can't unpreserve `x` because it was not being preserved.");
  }

  int n = pop_precious(stack);

  if (n < 0) {
    r_stop_internal("r_unpreserve", "`n` unexpectedly < 0.");
  }
  if (n == 0) {
    r_dict_del(&precious_dict, x);
  }
}


static
sexp* new_precious_stack(sexp* x) {
  sexp* stack = KEEP(r_new_list(2));

  // Store (0) protection count and (1) element to protect
  r_list_poke(stack, 0, r_int(0));
  r_list_poke(stack, 1, x);

  FREE(1);
  return stack;
}

static
int push_precious(sexp* stack) {
  sexp* n = r_list_get(stack, 0);
  int* p_n = r_int_deref(n);
  return ++(*p_n);
}

static
int pop_precious(sexp* stack) {
  sexp* n = r_list_get(stack, 0);
  int* p_n = r_int_deref(n);
  return --(*p_n);
}

// For unit tests
struct r_dict* rlang__precious_dict() {
  return &precious_dict;
}


void r_init_library_sexp(sexp* ns) {
  precious_dict = r_new_dict(PRECIOUS_DICT_INIT_SIZE);
  KEEP(precious_dict.shelter);
  r_env_poke(ns, r_sym(".__rlang_lib_precious_dict__."), precious_dict.shelter);
  FREE(1);
}
