/*
 * Add NetID and names of all project partners

Pavan Kumar Kokkiligadda : pkk46
Nicholas Chen : nhc29

 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BITMAP_SIZE 4    // size of the bitmap array
#define SET_BIT_INDEX 17 // bit index to set
#define GET_BIT_INDEX 17 // bit index to read

static unsigned int myaddress = 4026544704; // Binary  would be 11110000000000000011001001000000

/*
 * Function 1: FIND FIRST SET (FROM LSB) BIT
 */
static unsigned int first_set_bit(unsigned int num)
{
  // Implement your code here
  unsigned int location = 0;

  // from Brian kernighan's algorithm n&(n-1) flips rightmost set bit
  // n ^ (n&n-1) will isolate rightmost set bit
  // log2() gives position of first set bit
  location = (unsigned int)log2((num & num - 1) ^ num);
  return location;
}

/*
 * Function 2: SETTING A BIT AT AN INDEX
 * Function to set a bit at "index" bitmap
 */
static void set_bit_at_index(char *bitmap, int index)
{
  // Implement your code here
  int byte = index / 8;
  int bit = index % 8;

  bitmap[byte] |= (1 << bit);
  return;
}

/*
 * Function 3: GETTING A BIT AT AN INDEX
 * Function to get a bit at "index"
 */
static int get_bit_at_index(char *bitmap, int index)
{
  // Get to the location in the character bitmap array
  // Implement your code here
  int byte = index / 8;
  int bit = index % 8;

  return (bitmap[byte] >> bit) & 1;
}

int main()
{

  /*
   * Function 1: Finding the index of the first set bit (from LSB). Now let's say we
   * need to find the fsbl  of decimal 4026544704,  the fucntion should return 6.
   */
  unsigned int fsbl_value = first_set_bit(myaddress);
  printf("Function 1: first set bit value %u \n", fsbl_value);
  printf("\n");

  /*
   * Function 2 and 3: Checking if a bit is set or not
   */
  char *bitmap = (char *)malloc(BITMAP_SIZE); // We can store 32 bits (4*8-bit per character)
  memset(bitmap, 0, BITMAP_SIZE);             // clear everything

  /*
   * Let's try to set the bit
   */
  set_bit_at_index(bitmap, SET_BIT_INDEX);

  /*
   * Let's try to read bit)
   */
  printf("Function 3: The value at %dth location %d\n",
         GET_BIT_INDEX, get_bit_at_index(bitmap, GET_BIT_INDEX));

  return 0;
}
