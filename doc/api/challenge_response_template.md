

# <a href="response_to_challenge"/>response_to_challenge Computation

**Used to** :  
* compute response to given challenge to validate proper functioning of the CPU
* include index so the receiver can determine to which challenge this is a response
  * actual value is a 32-bit integer
  * stored in the Most Significant Bits of a 64-bit Unsigned Long (bits on LSB half should just be 0s for now)
  * transmitted as a String

**Data Type** : 64-bit Unsigned Long, transmitted as a String

**Formula** :  

    ((((Part1 * Part2) >> (Part3 && 0x0F)) + ((Part3 ^ Part1) << ((Part2 – Part1) && 0x07)))) / (0x03))

**Data** :  

| Index | Part 1 | Part 2 | Part 3 | Computed<br>Value | Response with Index<br>(use for response_to_challenge) |
| --- | --- | --- | --- | --- | --- |
| 0 | 2 | 10 | 1 | 4 | 4 |
| 1 | 2 | 255 | 250 | 2645 | 16779861 |
| 2 | 250 | 255 | 251 | 21 | 33554453 |
| 3 | 255 | 200 | 254 | 1 | 50331649 |

**Implementation in C** :

```c
/*  Function to calculate the response for given challenge
    It expects three uint8_t parts and one index as parameter

    The equation to calculate the response is:
    Response = ((((Part1 * Part2) >> (Part3 && 0x0F)) + ((Part3 ^ Part1) << ((Part2 � Part1) && 0x07)))) / (0x03))

    Function will return the last three bytes of response and index as MSB
*/
uint32_t cal_response(uint8_t part1, uint8_t part2, uint8_t part3,uint8_t idx)
{
    /*  We need to make sure the equation above is calculated in steps
        and avoid any language specific implicit data type conversions
        There could be several ways to do that, one way is listed here.
     */
    uint32_t tempval1 = 0;
    uint32_t tempval2 = 0;
    uint32_t tempval3 = 0;
    uint32_t calresp = 0;

    /* Will go from Left to right and calculate different intermediate values
       All intermediate results will be explicitly converted to uint32_t
       to avoid confusion of
     */
    /*  We know that two 8 bit values when multiplied can go above 2^8-1 range (255*255) */
    tempval1 = (uint32_t)((uint32_t)part1 * (uint32_t)part2);
    /*  We know that the bitwise and will never go above 8 bit size and this and operation
        will have maximum value of 0x0F and Min value of 0x00.
        So explicit conversion while performing the && is not required
        and even if we do tell compiler to do that, no harm
     */
    tempval2 = (uint32_t)(part3 & 0x0F);
    /* Now >> right shift is division by 2, so the values that we get by
       if above two values are its maximum will be (255*255) >> (0x0F) that will result in one
       printf("Result = 0x%08x\n",((255*255)>>0x0F));

       The max value of this operation can be (255*255) >> 0x00 when part3 is zero
       that will be 0x0000FE01

       in other cases it will result in integer division.
       That means if tempval2 is large and tempval1 is small - we will get result as zero
       And integer divisions are better compared to floating point division as its
       platform specific double precision/single precision etc

    */
    tempval3 = tempval1 >> tempval2;
    //printf("\nfirstpart: 0x%08x 0x%08x 0x%08x\n",tempval1,tempval2,tempval3);

    /*
       Now looking at the 3 part of the equation ^ bitwise XOR will
       never required to change the bit size. So result of this operation
       can not go above 0XFF. Explicit conversion not required, if do it then
       also no problem. Not that even order will not matter for Part1, Part3
     */
    tempval1 = (uint32_t)(part3 ^ part1);
    /*
      Now subtraction is the most worrysome bit, but the assumption here is
      even if we have signed representation of intermediate result, it uses
      modulo operation to rollover the values.
      printf("Implicit conversion Result = 0x%08x Explicit conversion result = 0x%08x\n",(0-5),(uint8_t)(0-5));

    */
    tempval2 = (uint32_t)((uint32_t)part2 - (uint32_t)part1);
    //printf("secondpart_1: 0x%08x 0x%08x\n",tempval1,tempval2);
    /*
        Now no matter what was the result of previous subtraction we are anding with LSB 4 bits
        That means the resulting value could not go above 0x07.
        Not that I have used the same variable on LHS - no harm here
     */
    tempval2 = (uint32_t)(tempval2 & (uint32_t)0x07);
    //printf("secondpart_2: 0x%08x 0x%08x\n",tempval1,tempval2);

    /*  Now calculate the shifted values
        This is the place where any conversion matters a lot.
        So explicit conversion is better to widen the result first
        Based on previously calculated results
        we can say that the worst case max value of this operation
        will be 0xFF << 0x07
        with conversion it will be 0x00007f80
    */
    tempval2 = (uint32_t) (tempval1 << tempval2);

    /* Now add two parts
        first part can have worst max value of 0x0000fe01
        second part can have worst max value of 0x00007f80
        so the addition of those two can not go above 0x00017d81
    */
    tempval3 = tempval3 + tempval2;

    /*
        Finally the division by 3 will always reduce the value back to
        first two bytes
    */
    calresp = (uint32_t) (tempval3/(uint32_t)0x03);

    /* add index value at MSB position */
    calresp = (calresp | (((uint32_t)idx) << (BITS_IN_BYTE*3)));

    return calresp;
}
```

