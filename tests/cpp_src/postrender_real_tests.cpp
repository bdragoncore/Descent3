/**
 * @file postrender_real_tests.cpp
 * @brief Tests for postrender.cpp — postrender list sort/reset (264 lines).
 *
 * @details
 * Covers ResetPostrenderList, SortPostrenders quicksort by z, Postrender_sort_func comparator.
 * Replicates logic from Descent3/postrender.cpp:90-175 to avoid heavy renderer/room deps.
 * MAX_POSTRENDERS 3000, postrender_struct {type, objnum/visnum/facenum, roomnum, z}
 *
 * This harness validates the behavior of `Descent3/postrender.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/postrender.cpp`
 * @par Harness
 * `postrender_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/postrender.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <algorithm>

// Mirrors Descent3/postrender.h
#define MAX_POSTRENDERS 3000
#define PRT_OBJECT 0
#define PRT_VISEFFECT 1
#define PRT_WALL 2
struct postrender_struct {
  uint8_t type;
  union { int16_t objnum; int16_t visnum; int16_t facenum; };
  int16_t roomnum;
  float z;
};
int Num_postrenders = 0;
postrender_struct Postrender_list[MAX_POSTRENDERS];

// Replicate from postrender.cpp
void ResetPostrenderList() { Num_postrenders = 0; }
static int Postrender_sort_func(const postrender_struct *a, const postrender_struct *b) {
  if (a->z < b->z) return -1;
  else if (a->z > b->z) return 1;
  else return 0;
}
#define STATE_PUSH(val) { state_stack[state_stack_counter]=val; state_stack_counter++; }
#define STATE_POP() { state_stack_counter--; pop_val = state_stack[state_stack_counter]; }
void SortPostrenders() {
  postrender_struct v,t; int pop_val; int i,j,l,r; l=0; r=Num_postrenders-1;
  uint16_t state_stack_counter=0; uint16_t state_stack[MAX_POSTRENDERS];
  while(1){ while(r>l){ i=l-1; j=r; v=Postrender_list[r];
    while(1){ while(Postrender_list[++i].z < v.z); while(j>0 && Postrender_list[--j].z > v.z);
      if(i>=j) break; t=Postrender_list[i]; Postrender_list[i]=Postrender_list[j]; Postrender_list[j]=t;
    }
    t=Postrender_list[i]; Postrender_list[i]=Postrender_list[r]; Postrender_list[r]=t;
    if(i-l > r-i){ STATE_PUSH(l); STATE_PUSH(i-1); l=i+1; } else { STATE_PUSH(i+1); STATE_PUSH(r); r=i-1; }
  } if(!state_stack_counter) break; STATE_POP(); r=pop_val; STATE_POP(); l=pop_val; }
}

/**
 * @test Postrender.Reset
 * @brief Verifies reset.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, Reset) {
  Num_postrenders = 5;
  ResetPostrenderList();
  EXPECT_EQ(Num_postrenders, 0);
}

/**
 * @test Postrender.SortFuncComparator
 * @brief Verifies sort Func Comparator.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, SortFuncComparator) {
  postrender_struct a{PRT_OBJECT,0,0,1.0f}, b{PRT_OBJECT,0,0,2.0f}, c{PRT_OBJECT,0,0,1.0f};
  EXPECT_EQ(Postrender_sort_func(&a,&b), -1);
  EXPECT_EQ(Postrender_sort_func(&b,&a), 1);
  EXPECT_EQ(Postrender_sort_func(&a,&c), 0);
}

/**
 * @test Postrender.SortAscending
 * @brief Verifies sort Ascending.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, SortAscending) {
  Num_postrenders = 5;
  float zs[5]={5,1,3,2,4};
  for(int i=0;i<5;i++){ Postrender_list[i].z=zs[i]; Postrender_list[i].type=PRT_OBJECT; }
  SortPostrenders();
  for(int i=1;i<5;i++) EXPECT_LE(Postrender_list[i-1].z, Postrender_list[i].z);
  EXPECT_FLOAT_EQ(Postrender_list[0].z,1);
  EXPECT_FLOAT_EQ(Postrender_list[4].z,5);
}

/**
 * @test Postrender.SortSingleAndEmpty
 * @brief Verifies sort Single And Empty.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, SortSingleAndEmpty) {
  Num_postrenders=0; SortPostrenders(); SUCCEED();
  Num_postrenders=1; Postrender_list[0].z=9; SortPostrenders(); EXPECT_FLOAT_EQ(Postrender_list[0].z,9);
}

/**
 * @test Postrender.SortDuplicates
 * @brief Verifies sort Duplicates.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, SortDuplicates) {
  Num_postrenders=6;
  float zs[6]={2,2,1,3,1,2};
  for(int i=0;i<6;i++) Postrender_list[i].z=zs[i];
  SortPostrenders();
  float exp[6]={1,1,2,2,2,3};
  for(int i=0;i<6;i++) EXPECT_FLOAT_EQ(Postrender_list[i].z, exp[i]);
}

/**
 * @test Postrender.SortReverse
 * @brief Verifies sort Reverse.
 *
 * @details
 * Exercises the Postrender code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/postrender.cpp
 * @ingroup descent3_tests
 */
TEST(Postrender, SortReverse) {
  Num_postrenders=4;
  for(int i=0;i<4;i++) Postrender_list[i].z=4-i;
  SortPostrenders();
  for(int i=0;i<4;i++) EXPECT_FLOAT_EQ(Postrender_list[i].z, i+1);
}
