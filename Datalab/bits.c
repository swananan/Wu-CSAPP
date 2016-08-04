/*
 * CS:APP Data Lab 
 *
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif

//bit部分！！
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) { 
    return ~((~x)|(~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
	return (x>>(n<<3))&0xff;

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
//利用掩码mask，把算术右移改成逻辑右移
int logicalShift(int x, int n) {
	int mask=~(1<<31);//mask为0x7fffffff
    mask=((mask>>n)<<1)+1;
	return (x>>n)&mask;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
//如果不限制操作的话，一个for循环就可以解决问题
//采用二分法，先计算x每两位中1的个数，并用对应的两队来存储
//这个个数。然后计算每4位1的个数，在用对应的4位进行存储。依次类推。
// 最后整合得到16位中1的个数，即x中的1的个数。	
int bitCount(int x) {
	int temp1=0x55|(0x55<<8);
	int mask1=temp1|(temp1<<16);//mask1  01010101 01010101 01010101 01010101
	int temp2=0x33|(0x33<<8);
	int mask2=temp2|(temp2<<16);//mask2  00110011 00110011 00110011 00110011
	int temp3=0x0f|(0x0f<<8);
	int mask3=temp3|(temp3<<16);//mask3  00001111 00001111 00001111 00001111
	int mask4=0xff|(0xff<<16);  //mask4  00000000 11111111 00000000 11111111
	int mask5=0xff|(0xff<<8);   //mask5  00000000 00000000 11111111 11111111
    int result;
    result=(x&mask1)+((x>>1)&mask1);
	result=(result&mask2)+((result>>2)&mask2);
	//result=(result&mask3)+((result>>4)&mask3);
	//result=(result&mask4)+((result>>8)&mask4);
	//result=(result&mask5)+((result>>16)&mask5);
    result=(result+(result>>4))&mask3;
    result=(result+(result>>8))&mask4;
    result=(result+(result>>16))&mask5;
	return result;
}
/*
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
//先位运算出x的负数，然后和x比较
int bang(int x) {
	int negativeX=(~x)+1;
	int temp=negativeX|x;
    temp=temp>>31;
	return temp+1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return 1<<31;
}

//补充一个函数
// 前面一部分用于判断左移一位后是否全部为0，后面一部分用来判断 x 值是否为 0
int isTmin(int x){
	return !(x+x)&!!(x);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
//要想x能被n位补码表示，则x的位数不能超过n位
//即判断x的位数大小
int fitsBits(int x, int n) {
    int shiftNum=32+(~n+1);//32-n
    return !(x^((x<<shiftNum)>>shiftNum));
}




//整数部分！！
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
//x大于0的时候，可以直接x>>n，来表示除法
//但是x小于0的时候，不能保证向0取整，要给x加上一个（1<<n）-1的偏置
int divpwr2(int x, int n) {
	int sign=x>>31;//算术右移
	int mask=(1<<n)+(~0);//~0为32个1 。得到2^n-1，即偏置
	int bias=sign &mask;
	return (x+bias)>>n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
	return ~x+1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
	int sign=x>>31;	
	return !(sign)&!!(x);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int signX=(x>>31)&1;//获取符号位
    int signY=(y>>31)&1;
    int temp=x+((~y));//temp为同号情况下的x-y的值.
    int signSame=(!(signX^signY))&(temp>>31);//这是同号情况下，根据temp的符号位来判断x与y的大小
    int signDiff=signX&(!signY);//当x为正，y为负时，signdiff为1，即x>y
    return signSame|signDiff;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0//默认x大于0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
//即找第一个为1的比特位在哪的问题
//利用二分法找，时间复杂度O(lgn)
int ilog2(int x) {
    int bitsNumber=(!!(x>>16))<<4;//若x在16到31位中有值，则bitsnumber为16
    bitsNumber=bitsNumber+((!!(x>>(bitsNumber+8)))<<3);
    bitsNumber=bitsNumber+((!!(x>>(bitsNumber+4)))<<2);
    bitsNumber=bitsNumber+((!!(x>>(bitsNumber+2)))<<1);
    bitsNumber=bitsNumber+(!!(x>>(bitsNumber+1)));
    //由于限制了操作，只能用加法来表示
    bitsNumber=bitsNumber+(!!bitsNumber)+(~0)+(!(1^x));//x已经在之前被>>16改变了
    return bitsNumber;
}




//浮点数部分！
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
//将以unsigned int形式传进来的参数，返回对应浮点数参数-f相等的二进制值
//终于可以使用条件判断语句和循环语句了，憋死了
unsigned float_neg(unsigned uf) {
    unsigned temp=uf&0x7fffffff;//字面常量的限制也没有了
    if(temp>0x7f800000)return uf;//NAN的情况
    return uf^0x80000000;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
//将x的值当做浮点数的值，翻译成二进制并且返回
//返回结果 (sign<<31)|(exponent<<23)|fraction
unsigned float_i2f(int x) {
    if(!x)return x;
    int sign=x>>31&1;
    int exponent;
    int fraction;
    int fraction_mask;
    int bias=127;//float的指数位偏置
    if(sign)x=-x;//保证x值为正数
    int i=30;//i为x究竟有多少比特位
    while (!(x>>i)) {
        i=i-1;
    }
    exponent=bias+i;
    if (i<=23) {
        fraction=x<<(23-i);
    }
    else{
        fraction=x>>(i-23);
        fraction_mask=(1<<(i-23))-1;
        //这里要考虑舍入情况
        if ((x&fraction_mask)>(1<<(i-24))) {
            fraction=fraction+1;
        }
        if ((x&fraction_mask)==(1<<(i-24))&&(fraction&1)) {
            fraction=fraction+1;
        }
        if (fraction==(1<<24)) {
            exponent=exponent+1;
        }
    }
    return (sign<<31)|(exponent<<23)|(fraction&0x7fffff);
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
//
unsigned float_twice(unsigned uf) {
    unsigned result=uf;
    unsigned sign=(uf>>31)<<31;
    unsigned exponent=(uf-sign)>>23;
    //当指数位exponent为11111111时,即2f为NAN
    if(!(exponent^0xff))return result;
    //当指数位exponent为11111110时，返回无穷
    else if(!(0xff-exponent-1))return sign|(0xff<<23);
    else if(exponent)return result+0x00800000;
    return sign|(result<<1);
}
