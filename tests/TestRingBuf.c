#include "unity.h"
#include "RingBuf.h"

uint8_t *data;

void test_buffer_empty(void) {
   RINGBUF_DEF(empty_buf,32);
   TEST_ASSERT_EQUAL_INT(-1,ringBufPop(&empty_buf, data));
}

void test_buffer_push (void) {
    RINGBUF_DEF(empty_buf,32);
    ringBufPush(&empty_buf,11);
    TEST_ASSERT_EQUAL_INT(11,empty_buf.buffer[0]);
    ringBufPush(&empty_buf,12);
    TEST_ASSERT_EQUAL_INT(11,empty_buf.buffer[0]);
    TEST_ASSERT_EQUAL_INT(12,empty_buf.buffer[1]);
}

void test_buffer_full(void) {
    RINGBUF_DEF(buf_2,2);
    ringBufPush(&buf_2,1);
    TEST_ASSERT_EQUAL_INT(-1,ringBufPush(&buf_2,2));
    TEST_ASSERT_EQUAL_INT(-1,ringBufPush(&buf_2,3));
    TEST_ASSERT_EQUAL_INT(-1,ringBufPush(&buf_2,4));
    TEST_ASSERT_EQUAL_INT(1,buf_2.buffer[0]);
}

void test_buffer_pop(void) {
    uint8_t data;
    RINGBUF_DEF(buf_3,3);
    TEST_ASSERT_EQUAL_INT(-1,ringBufPop(&buf_3,&data));
    ringBufPush(&buf_3,1);
    ringBufPush(&buf_3,2);
    TEST_ASSERT_EQUAL_INT(-1,ringBufPush(&buf_3,3)); 
    ringBufPop(&buf_3,&data);
    TEST_ASSERT_EQUAL_INT(1,data);
    ringBufPop(&buf_3,&data);
    TEST_ASSERT_EQUAL_INT(2,data);
    TEST_ASSERT_EQUAL_INT(0,ringBufPush(&buf_3,3));
    ringBufPop(&buf_3,&data);
    TEST_ASSERT_EQUAL_INT(3,data);
}

int main(void)
{
UNITY_BEGIN();
RUN_TEST(test_buffer_empty);
RUN_TEST(test_buffer_push);
RUN_TEST(test_buffer_full);
RUN_TEST(test_buffer_pop);
return UNITY_END();
}
