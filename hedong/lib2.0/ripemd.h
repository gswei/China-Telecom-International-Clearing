#ifndef Goodzzp_RipeMd_05_10_20_12
#define Goodzzp_RipeMd_05_10_20_12

//ripemd128,160,256,320
//program be Goodzzp
//05,10,20
//128 and 160 are referred:
//http://homes.esat.md_kuleuven.be/~bosselae/ripemd160.html 
//256 and 320 are not guarranteed to be safe than 128 or 160
//referred: Crypto++5.21

#include "define.h"

//128-320需要的5论变换
#define md_F(x, y, z)    (x ^ y ^ z) 
#define md_G(x, y, z)    (z ^ (x & (y^z)))
#define md_H(x, y, z)    (z ^ (x | ~y))
#define md_I(x, y, z)    (y ^ (z & (x^y)))
#define md_J(x, y, z)    (x ^ (y | ~z))
//常量
#define md_k0 0
#define md_k1 0x5a827999UL
#define md_k2 0x6ed9eba1UL
#define md_k3 0x8f1bbcdcUL
#define md_k4 0xa953fd4eUL
#define md_k5 0x50a28be6UL
#define md_k6 0x5c4dd124UL
#define md_k7 0x6d703ef3UL
#define md_k8 0x7a6d76e9UL
#define md_k9 0  
// for 160 and 320
#define md_Subround(f, a, b, c, d, e, x, s, md_k)        \
	a += f(b, c, d) + x + md_k;\
	a = rotlFixed((UI32)a, s) + e;\
	c = rotlFixed((UI32)c, 10U)
// for 128 and 256
#define md1_Subround(f, a, b, c, d, x, s, md_k)        \
	a += f(b, c, d) + x + md_k;\
	a = rotlFixed((UI32)a, s);

//...begin ripemd128

//RipeMd128散列类
class RipeMD128
{
public:
	//输出结果的长度（字节）
	static UI32 OutDataLength()
	{
		return 128/8;
	}
	//RipeMD128变换函数
	//out:输出，长度为16，要求事先已经分配了内存
	//in:输入
	//length:输入值的长度
	void Hash(UI8 *out,const UI8 *in,UI32 length)
	{
		UI32 i = length>>6,j=(length&0x3f),md_k;
		//对数据64 64个字节的计算
		for(md_k=0;md_k<i;md_k++)
		{
			StepTransform((UI8 *)(in + md_k * 64),64,length);
		}
		//最后计算尾数
		StepTransform((UI8 *)(in + 64 * i),j,length);
		//拷贝输出
		memcpy(out,MDbuf,16);
		//恢复m_state值
		MDbuf[0] = 0x67452301UL;
		MDbuf[1] = 0xefcdab89UL;
		MDbuf[2] = 0x98badcfeUL;
		MDbuf[3] = 0x10325476UL;
	}
	//初始化
	//这里就是4个刚开始的值
	RipeMD128()
	{
		MDbuf[0] = 0x67452301UL;
		MDbuf[1] = 0xefcdab89UL;
		MDbuf[2] = 0x98badcfeUL;
		MDbuf[3] = 0x10325476UL;
	}
private:
	//每步的变换函数
	//输入:
	//		data:			要处理的数据块（不大于64字节）
	//		dataBlockLen:	数据块的长度
	//		dataTotalLen:	要处理的所有数据块的总长度
	//输出结果保存在m_state里面
	void StepTransform(UI8 *data,UI32 dataBlockLen, UI32 dataTotalLen)
	{
		UI8 buffer[64];
		UI32 len=dataTotalLen*8;

		memset(buffer,0,64);//清空数据为0
		memcpy(buffer,data,dataBlockLen);//拷贝数据到缓冲

		if(dataBlockLen <64)	//需要增加数据
		{
			if(dataBlockLen<56)//当前数据是最后若干个，而且不需要增加一次变换
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//添加长度
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
			else if(dataBlockLen>=56)
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
				//添加长度
				memset(buffer,0,64);
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
		}
		else if(dataBlockLen == 64)
		{
			//变换
			FirstTransform((UI32*)buffer);
			CoreTransform();
		}
	}

	//把64字节的原始数据data进行初步转化到m_data中去
	void FirstTransform(UI32 *data)
	{
		memcpy(X,data,64);
	}
	
	//核心变换
	void CoreTransform()
	{
		UI32 a1, b1, c1, d1, a2, b2, c2, d2;
		a1 = a2 = MDbuf[0];
		b1 = b2 = MDbuf[1];
		c1 = c2 = MDbuf[2];
		d1 = d2 = MDbuf[3];

		md1_Subround(md_F, a1, b1, c1, d1, X[ 0], 11, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 1], 14, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[ 2], 15, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[ 3], 12, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[ 4],  5, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 5],  8, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[ 6],  7, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[ 7],  9, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[ 8], 11, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 9], 13, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[10], 14, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[11], 15, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[12],  6, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[13],  7, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[14],  9, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[15],  8, md_k0);

		md1_Subround(md_G, a1, b1, c1, d1, X[ 7],  7, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 4],  6, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[13],  8, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 1], 13, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[10], 11, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 6],  9, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[15],  7, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 3], 15, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[12],  7, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 0], 12, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[ 9], 15, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 5],  9, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[ 2], 11, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[14],  7, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[11], 13, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 8], 12, md_k1);

		md1_Subround(md_H, a1, b1, c1, d1, X[ 3], 11, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[10], 13, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[14],  6, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 4],  7, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[ 9], 14, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[15],  9, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 8], 13, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 1], 15, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[ 2], 14, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[ 7],  8, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 0], 13, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 6],  6, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[13],  5, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[11], 12, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 5],  7, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[12],  5, md_k2);

		md1_Subround(md_I, a1, b1, c1, d1, X[ 1], 11, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 9], 12, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[11], 14, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[10], 15, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[ 0], 14, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 8], 15, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[12],  9, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[ 4],  8, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[13],  9, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 3], 14, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[ 7],  5, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[15],  6, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[14],  8, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 5],  6, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[ 6],  5, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[ 2], 12, md_k3);

		md1_Subround(md_I, a2, b2, c2, d2, X[ 5],  8, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[14],  9, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[ 7],  9, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 0], 11, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[ 9], 13, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[ 2], 15, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[11], 15, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 4],  5, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[13],  7, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[ 6],  7, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[15],  8, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 8], 11, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[ 1], 14, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[10], 14, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[ 3], 12, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[12],  6, md_k5);

		md1_Subround(md_H, a2, b2, c2, d2, X[ 6],  9, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[11], 13, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 3], 15, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[ 7],  7, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[ 0], 12, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[13],  8, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 5],  9, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[10], 11, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[14],  7, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[15],  7, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 8], 12, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[12],  7, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[ 4],  6, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[ 9], 15, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 1], 13, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[ 2], 11, md_k6);

		md1_Subround(md_G, a2, b2, c2, d2, X[15],  9, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 5],  7, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 1], 15, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 3], 11, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[ 7],  8, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[14],  6, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 6],  6, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 9], 14, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[11], 12, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 8], 13, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[12],  5, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 2], 14, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[10], 13, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 0], 13, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 4],  7, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[13],  5, md_k7);

		md1_Subround(md_F, a2, b2, c2, d2, X[ 8], 15, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[ 6],  5, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[ 4],  8, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[ 1], 11, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 3], 14, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[11], 14, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[15],  6, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[ 0], 14, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 5],  6, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[12],  9, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[ 2], 12, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[13],  9, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 9], 12, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[ 7],  5, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[10], 15, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[14],  8, md_k9);

		c1        = MDbuf[1] + c1 + d2;
		MDbuf[1] = MDbuf[2] + d1 + a2;
		MDbuf[2] = MDbuf[3] + a1 + b2;
		MDbuf[3] = MDbuf[0] + b1 + c2;
		MDbuf[0] = c1;
	}

private:
	UI32 MDbuf[4];		//保存有ripemd输出值
	UI32 X[16];			//保存有每步变换时对输入的初始化转化值
};
//...end ripemd128

//...begin ripeMd160

//RipeMd160散列类
class RipeMD160
{
public:
	//输出结果的长度（字节）
	static UI32 OutDataLength()
	{
		return 160/8;
	}
	//RipeMD160变换函数
	//out:输出，长度为20，要求事先已经分配了内存
	//in:输入
	//length:输入值的长度
	void Hash(UI8 *out,const UI8 *in,UI32 length)
	{
		UI32 i = length>>6,j=(length&0x3f),md_k;
		//对数据64 64个字节的计算
		for(md_k=0;md_k<i;md_k++)
		{
			StepTransform((UI8 *)(in + md_k * 64),64,length);
		}
		//最后计算尾数
		StepTransform((UI8 *)(in + 64 * i),j,length);
		//拷贝输出
		memcpy(out,MDbuf,20);
		//恢复MDbuf值
		MDbuf[0] = 0x67452301UL;
		MDbuf[1] = 0xefcdab89UL;
		MDbuf[2] = 0x98badcfeUL;
		MDbuf[3] = 0x10325476UL;
		MDbuf[4] = 0xc3d2e1f0UL;
	}
	//初始化
	//这里就是5个刚开始的值
	RipeMD160()
	{
		MDbuf[0] = 0x67452301UL;
		MDbuf[1] = 0xefcdab89UL;
		MDbuf[2] = 0x98badcfeUL;
		MDbuf[3] = 0x10325476UL;
		MDbuf[4] = 0xc3d2e1f0UL;
	}
private:
	//每步的变换函数
	//输入:
	//		data:			要处理的数据块（不大于64字节）
	//		dataBlockLen:	数据块的长度
	//		dataTotalLen:	要处理的所有数据块的总长度
	//输出结果保存在m_state里面
	void StepTransform(UI8 *data,UI32 dataBlockLen, UI32 dataTotalLen)
	{
		UI8 buffer[64];
		UI32 len=dataTotalLen*8;

		memset(buffer,0,64);//清空数据为0
		memcpy(buffer,data,dataBlockLen);//拷贝数据到缓冲

		if(dataBlockLen <64)	//需要增加数据
		{
			if(dataBlockLen<56)//当前数据是最后若干个，而且不需要增加一次变换
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//添加长度
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
			else if(dataBlockLen>=56)
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
				//添加长度
				memset(buffer,0,64);
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
		}
		else if(dataBlockLen == 64)
		{
			//变换
			FirstTransform((UI32*)buffer);
			CoreTransform();
		}
	}

	//把64字节的原始数据data进行初步转化到m_data中去
	void FirstTransform(UI32 *data)
	{
		memcpy(X,data,64);
	}
	
	//核心变换
	void CoreTransform()
	{
		UI32 a1, b1, c1, d1, e1, a2, b2, c2, d2, e2;
		a1 = a2 = MDbuf[0];
		b1 = b2 = MDbuf[1];
		c1 = c2 = MDbuf[2];
		d1 = d2 = MDbuf[3];
		e1 = e2 = MDbuf[4];

		md_Subround(md_F, a1, b1, c1, d1, e1, X[ 0], 11, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[ 1], 14, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[ 2], 15, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[ 3], 12, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[ 4],  5, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[ 5],  8, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[ 6],  7, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[ 7],  9, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[ 8], 11, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[ 9], 13, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[10], 14, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[11], 15, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[12],  6, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[13],  7, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[14],  9, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[15],  8, md_k0);

		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 7],  7, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[ 4],  6, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[13],  8, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[ 1], 13, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[10], 11, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 6],  9, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[15],  7, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[ 3], 15, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[12],  7, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[ 0], 12, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 9], 15, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[ 5],  9, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[ 2], 11, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[14],  7, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[11], 13, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 8], 12, md_k1);

		md_Subround(md_H, d1, e1, a1, b1, c1, X[ 3], 11, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[10], 13, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[14],  6, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[ 4],  7, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 9], 14, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[15],  9, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[ 8], 13, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[ 1], 15, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[ 2], 14, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 7],  8, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[ 0], 13, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[ 6],  6, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[13],  5, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[11], 12, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 5],  7, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[12],  5, md_k2);

		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 1], 11, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[ 9], 12, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[11], 14, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[10], 15, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 0], 14, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 8], 15, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[12],  9, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[ 4],  8, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[13],  9, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 3], 14, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 7],  5, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[15],  6, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[14],  8, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[ 5],  6, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 6],  5, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 2], 12, md_k3);

		md_Subround(md_J, b1, c1, d1, e1, a1, X[ 4],  9, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 0], 15, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[ 5],  5, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[ 9], 11, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[ 7],  6, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[12],  8, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 2], 13, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[10], 12, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[14],  5, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[ 1], 12, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[ 3], 13, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 8], 14, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[11], 11, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[ 6],  8, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[15],  5, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[13],  6, md_k4);

		md_Subround(md_J, a2, b2, c2, d2, e2, X[ 5],  8, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[14],  9, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 7],  9, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[ 0], 11, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 9], 13, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[ 2], 15, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[11], 15, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 4],  5, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[13],  7, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 6],  7, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[15],  8, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[ 8], 11, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 1], 14, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[10], 14, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 3], 12, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[12],  6, md_k5);

		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 6],  9, md_k6); 
		md_Subround(md_I, d2, e2, a2, b2, c2, X[11], 13, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[ 3], 15, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[ 7],  7, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[ 0], 12, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[13],  8, md_k6);
		md_Subround(md_I, d2, e2, a2, b2, c2, X[ 5],  9, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[10], 11, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[14],  7, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[15],  7, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 8], 12, md_k6);
		md_Subround(md_I, d2, e2, a2, b2, c2, X[12],  7, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[ 4],  6, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[ 9], 15, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[ 1], 13, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 2], 11, md_k6);

		md_Subround(md_H, d2, e2, a2, b2, c2, X[15],  9, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 5],  7, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[ 1], 15, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[ 3], 11, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 7],  8, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[14],  6, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 6],  6, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[ 9], 14, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[11], 12, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 8], 13, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[12],  5, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 2], 14, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[10], 13, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[ 0], 13, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 4],  7, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[13],  5, md_k7);

		md_Subround(md_G, c2, d2, e2, a2, b2, X[ 8], 15, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[ 6],  5, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 4],  8, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 1], 11, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[ 3], 14, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[11], 14, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[15],  6, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 0], 14, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 5],  6, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[12],  9, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[ 2], 12, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[13],  9, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 9], 12, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 7],  5, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[10], 15, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[14],  8, md_k8);

		md_Subround(md_F, b2, c2, d2, e2, a2, X[12],  8, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[15],  5, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[10], 12, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 4],  9, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 1], 12, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[ 5],  5, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[ 8], 14, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[ 7],  6, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 6],  8, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 2], 13, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[13],  6, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[14],  5, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[ 0], 15, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 3], 13, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 9], 11, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[11], 11, md_k9);

		c1        = MDbuf[1] + c1 + d2;
		MDbuf[1] = MDbuf[2] + d1 + e2;
		MDbuf[2] = MDbuf[3] + e1 + a2;
		MDbuf[3] = MDbuf[4] + a1 + b2;
		MDbuf[4] = MDbuf[0] + b1 + c2;
		MDbuf[0] = c1;
	}

private:
	UI32 MDbuf[5];		//保存有ripemd输出值
	UI32 X[16];			//保存有每步变换时对输入的初始化转化值
};
//...end ripeMd160

//...begin ripemd256
//RipeMd256散列类
class RipeMD256
{
public:
	//输出结果的长度（字节）
	static UI32 OutDataLength()
	{
		return 256/8;
	}
	//RipeMD256变换函数
	//out:输出，长度为32，要求事先已经分配了内存
	//in:输入
	//length:输入值的长度
	void Hash(UI8 *out,const UI8 *in,UI32 length)
	{
		UI32 i = length>>6,j=(length&0x3f),md_k;
		//对数据64 64个字节的计算
		for(md_k=0;md_k<i;md_k++)
		{
			StepTransform((UI8 *)(in + md_k * 64),64,length);
		}
		//最后计算尾数
		StepTransform((UI8 *)(in + 64 * i),j,length);
		//拷贝输出
		memcpy(out,MDbuf,32);
		//恢复MDbuf值
		MDbuf[0] = 0x67452301L;
		MDbuf[1] = 0xefcdab89L;
		MDbuf[2] = 0x98badcfeL;
		MDbuf[3] = 0x10325476L;
		MDbuf[4] = 0x76543210L;
		MDbuf[5] = 0xfedcba98L;
		MDbuf[6] = 0x89abcdefL;
		MDbuf[7] = 0x01234567L;
	}
	//初始化
	//这里就是8个刚开始的值
	RipeMD256()
	{
		MDbuf[0] = 0x67452301L;
		MDbuf[1] = 0xefcdab89L;
		MDbuf[2] = 0x98badcfeL;
		MDbuf[3] = 0x10325476L;
		MDbuf[4] = 0x76543210L;
		MDbuf[5] = 0xfedcba98L;
		MDbuf[6] = 0x89abcdefL;
		MDbuf[7] = 0x01234567L;
	}
private:
	//每步的变换函数
	//输入:
	//		data:			要处理的数据块（不大于64字节）
	//		dataBlockLen:	数据块的长度
	//		dataTotalLen:	要处理的所有数据块的总长度
	//输出结果保存在m_state里面
	void StepTransform(UI8 *data,UI32 dataBlockLen, UI32 dataTotalLen)
	{
		UI8 buffer[64];
		UI32 len=dataTotalLen*8;

		memset(buffer,0,64);//清空数据为0
		memcpy(buffer,data,dataBlockLen);//拷贝数据到缓冲

		if(dataBlockLen <64)	//需要增加数据
		{
			if(dataBlockLen<56)//当前数据是最后若干个，而且不需要增加一次变换
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//添加长度
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
			else if(dataBlockLen>=56)
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
				//添加长度
				memset(buffer,0,64);
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
		}
		else if(dataBlockLen == 64)
		{
			//变换
			FirstTransform((UI32*)buffer);
			CoreTransform();
		}
	}

	//把64字节的原始数据data进行初步转化到m_data中去
	void FirstTransform(UI32 *data)
	{
		memcpy(X,data,64);
	}
	
	//核心变换
	void CoreTransform()
	{
		UI32 a1, b1, c1, d1, a2, b2, c2, d2, t;
		a1 = MDbuf[0];
		b1 = MDbuf[1];
		c1 = MDbuf[2];
		d1 = MDbuf[3];
		a2 = MDbuf[4];
		b2 = MDbuf[5];
		c2 = MDbuf[6];
		d2 = MDbuf[7];

		md1_Subround(md_F, a1, b1, c1, d1, X[ 0], 11, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 1], 14, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[ 2], 15, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[ 3], 12, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[ 4],  5, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 5],  8, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[ 6],  7, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[ 7],  9, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[ 8], 11, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[ 9], 13, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[10], 14, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[11], 15, md_k0);
		md1_Subround(md_F, a1, b1, c1, d1, X[12],  6, md_k0);
		md1_Subround(md_F, d1, a1, b1, c1, X[13],  7, md_k0);
		md1_Subround(md_F, c1, d1, a1, b1, X[14],  9, md_k0);
		md1_Subround(md_F, b1, c1, d1, a1, X[15],  8, md_k0);

		md1_Subround(md_I, a2, b2, c2, d2, X[ 5],  8, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[14],  9, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[ 7],  9, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 0], 11, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[ 9], 13, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[ 2], 15, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[11], 15, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 4],  5, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[13],  7, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[ 6],  7, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[15],  8, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[ 8], 11, md_k5);
		md1_Subround(md_I, a2, b2, c2, d2, X[ 1], 14, md_k5);
		md1_Subround(md_I, d2, a2, b2, c2, X[10], 14, md_k5);
		md1_Subround(md_I, c2, d2, a2, b2, X[ 3], 12, md_k5);
		md1_Subround(md_I, b2, c2, d2, a2, X[12],  6, md_k5);

		t = a1; a1 = a2; a2 = t;

		md1_Subround(md_G, a1, b1, c1, d1, X[ 7],  7, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 4],  6, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[13],  8, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 1], 13, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[10], 11, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 6],  9, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[15],  7, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 3], 15, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[12],  7, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[ 0], 12, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[ 9], 15, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 5],  9, md_k1);
		md1_Subround(md_G, a1, b1, c1, d1, X[ 2], 11, md_k1);
		md1_Subround(md_G, d1, a1, b1, c1, X[14],  7, md_k1);
		md1_Subround(md_G, c1, d1, a1, b1, X[11], 13, md_k1);
		md1_Subround(md_G, b1, c1, d1, a1, X[ 8], 12, md_k1);

		md1_Subround(md_H, a2, b2, c2, d2, X[ 6],  9, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[11], 13, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 3], 15, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[ 7],  7, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[ 0], 12, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[13],  8, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 5],  9, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[10], 11, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[14],  7, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[15],  7, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 8], 12, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[12],  7, md_k6);
		md1_Subround(md_H, a2, b2, c2, d2, X[ 4],  6, md_k6);
		md1_Subround(md_H, d2, a2, b2, c2, X[ 9], 15, md_k6);
		md1_Subround(md_H, c2, d2, a2, b2, X[ 1], 13, md_k6);
		md1_Subround(md_H, b2, c2, d2, a2, X[ 2], 11, md_k6);

		t = b1; b1 = b2; b2 = t;

		md1_Subround(md_H, a1, b1, c1, d1, X[ 3], 11, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[10], 13, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[14],  6, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 4],  7, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[ 9], 14, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[15],  9, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 8], 13, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 1], 15, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[ 2], 14, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[ 7],  8, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 0], 13, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[ 6],  6, md_k2);
		md1_Subround(md_H, a1, b1, c1, d1, X[13],  5, md_k2);
		md1_Subround(md_H, d1, a1, b1, c1, X[11], 12, md_k2);
		md1_Subround(md_H, c1, d1, a1, b1, X[ 5],  7, md_k2);
		md1_Subround(md_H, b1, c1, d1, a1, X[12],  5, md_k2);

		md1_Subround(md_G, a2, b2, c2, d2, X[15],  9, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 5],  7, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 1], 15, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 3], 11, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[ 7],  8, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[14],  6, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 6],  6, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 9], 14, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[11], 12, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 8], 13, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[12],  5, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[ 2], 14, md_k7);
		md1_Subround(md_G, a2, b2, c2, d2, X[10], 13, md_k7);
		md1_Subround(md_G, d2, a2, b2, c2, X[ 0], 13, md_k7);
		md1_Subround(md_G, c2, d2, a2, b2, X[ 4],  7, md_k7);
		md1_Subround(md_G, b2, c2, d2, a2, X[13],  5, md_k7);

		t = c1; c1 = c2; c2 = t;

		md1_Subround(md_I, a1, b1, c1, d1, X[ 1], 11, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 9], 12, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[11], 14, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[10], 15, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[ 0], 14, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 8], 15, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[12],  9, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[ 4],  8, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[13],  9, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 3], 14, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[ 7],  5, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[15],  6, md_k3);
		md1_Subround(md_I, a1, b1, c1, d1, X[14],  8, md_k3);
		md1_Subround(md_I, d1, a1, b1, c1, X[ 5],  6, md_k3);
		md1_Subround(md_I, c1, d1, a1, b1, X[ 6],  5, md_k3);
		md1_Subround(md_I, b1, c1, d1, a1, X[ 2], 12, md_k3);

		md1_Subround(md_F, a2, b2, c2, d2, X[ 8], 15, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[ 6],  5, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[ 4],  8, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[ 1], 11, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 3], 14, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[11], 14, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[15],  6, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[ 0], 14, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 5],  6, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[12],  9, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[ 2], 12, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[13],  9, md_k9);
		md1_Subround(md_F, a2, b2, c2, d2, X[ 9], 12, md_k9);
		md1_Subround(md_F, d2, a2, b2, c2, X[ 7],  5, md_k9);
		md1_Subround(md_F, c2, d2, a2, b2, X[10], 15, md_k9);
		md1_Subround(md_F, b2, c2, d2, a2, X[14],  8, md_k9);

		t = d1; d1 = d2; d2 = t;

		MDbuf[0] += a1;
		MDbuf[1] += b1;
		MDbuf[2] += c1;
		MDbuf[3] += d1;
		MDbuf[4] += a2;
		MDbuf[5] += b2;
		MDbuf[6] += c2;
		MDbuf[7] += d2;
	}

private:
	UI32 MDbuf[8];		//保存有ripemd输出值
	UI32 X[16];			//保存有每步变换时对输入的初始化转化值
};
//...end ripeMd256

//...begin ripeMd320
//RipeMd320散列类
class RipeMD320
{
public:
	//输出结果的长度（字节）
	static UI32 OutDataLength()
	{
		return 320/8;
	}
	//RipeMD320变换函数
	//out:输出，长度为40，要求事先已经分配了内存
	//in:输入
	//length:输入值的长度
	void Hash(UI8 *out,const UI8 *in,UI32 length)
	{
		UI32 i = length>>6,j=(length&0x3f),md_k;
		//对数据64 64个字节的计算
		for(md_k=0;md_k<i;md_k++)
		{
			StepTransform((UI8 *)(in + md_k * 64),64,length);
		}
		//最后计算尾数
		StepTransform((UI8 *)(in + 64 * i),j,length);
		//拷贝输出
		memcpy(out,MDbuf,40);
		//恢复MDbuf值
		MDbuf[0] = 0x67452301L;
		MDbuf[1] = 0xefcdab89L;
		MDbuf[2] = 0x98badcfeL;
		MDbuf[3] = 0x10325476L;
		MDbuf[4] = 0xc3d2e1f0L;
		MDbuf[5] = 0x76543210L;
		MDbuf[6] = 0xfedcba98L;
		MDbuf[7] = 0x89abcdefL;
		MDbuf[8] = 0x01234567L;
		MDbuf[9] = 0x3c2d1e0fL;
	}
	//初始化
	//这里就是10个刚开始的值
	RipeMD320()
	{
		MDbuf[0] = 0x67452301L;
		MDbuf[1] = 0xefcdab89L;
		MDbuf[2] = 0x98badcfeL;
		MDbuf[3] = 0x10325476L;
		MDbuf[4] = 0xc3d2e1f0L;
		MDbuf[5] = 0x76543210L;
		MDbuf[6] = 0xfedcba98L;
		MDbuf[7] = 0x89abcdefL;
		MDbuf[8] = 0x01234567L;
		MDbuf[9] = 0x3c2d1e0fL;
	}
private:
	//每步的变换函数
	//输入:
	//		data:			要处理的数据块（不大于64字节）
	//		dataBlockLen:	数据块的长度
	//		dataTotalLen:	要处理的所有数据块的总长度
	//输出结果保存在m_state里面
	void StepTransform(UI8 *data,UI32 dataBlockLen, UI32 dataTotalLen)
	{
		UI8 buffer[64];
		UI32 len=dataTotalLen*8;

		memset(buffer,0,64);//清空数据为0
		memcpy(buffer,data,dataBlockLen);//拷贝数据到缓冲

		if(dataBlockLen <64)	//需要增加数据
		{
			if(dataBlockLen<56)//当前数据是最后若干个，而且不需要增加一次变换
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//添加长度
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
			else if(dataBlockLen>=56)
			{
				//添加1和0
				buffer[dataBlockLen]=0x80;
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
				//添加长度
				memset(buffer,0,64);
				buffer[56]=(UI8)(len&0xff);
				len>>=8;
				buffer[57]=(UI8)(len&0xff);
				len>>=8;
				buffer[58]=(UI8)(len&0xff);
				len>>=8;
				buffer[59]=(UI8)(len&0xff);
				//变换
				FirstTransform((UI32*)buffer);
				CoreTransform();
			}
		}
		else if(dataBlockLen == 64)
		{
			//变换
			FirstTransform((UI32*)buffer);
			CoreTransform();
		}
	}

	//把64字节的原始数据data进行初步转化到m_data中去
	void FirstTransform(UI32 *data)
	{
		memcpy(X,data,64);
	}
	
	//核心变换
	void CoreTransform()
	{
		UI32 a1, b1, c1, d1, e1, a2, b2, c2, d2, e2, t;
		a1 = MDbuf[0];
		b1 = MDbuf[1];
		c1 = MDbuf[2];
		d1 = MDbuf[3];
		e1 = MDbuf[4];
		a2 = MDbuf[5];
		b2 = MDbuf[6];
		c2 = MDbuf[7];
		d2 = MDbuf[8];
		e2 = MDbuf[9];

		md_Subround(md_F, a1, b1, c1, d1, e1, X[ 0], 11, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[ 1], 14, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[ 2], 15, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[ 3], 12, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[ 4],  5, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[ 5],  8, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[ 6],  7, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[ 7],  9, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[ 8], 11, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[ 9], 13, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[10], 14, md_k0);
		md_Subround(md_F, e1, a1, b1, c1, d1, X[11], 15, md_k0);
		md_Subround(md_F, d1, e1, a1, b1, c1, X[12],  6, md_k0);
		md_Subround(md_F, c1, d1, e1, a1, b1, X[13],  7, md_k0);
		md_Subround(md_F, b1, c1, d1, e1, a1, X[14],  9, md_k0);
		md_Subround(md_F, a1, b1, c1, d1, e1, X[15],  8, md_k0);

		md_Subround(md_J, a2, b2, c2, d2, e2, X[ 5],  8, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[14],  9, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 7],  9, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[ 0], 11, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 9], 13, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[ 2], 15, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[11], 15, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 4],  5, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[13],  7, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 6],  7, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[15],  8, md_k5);
		md_Subround(md_J, e2, a2, b2, c2, d2, X[ 8], 11, md_k5);
		md_Subround(md_J, d2, e2, a2, b2, c2, X[ 1], 14, md_k5);
		md_Subround(md_J, c2, d2, e2, a2, b2, X[10], 14, md_k5);
		md_Subround(md_J, b2, c2, d2, e2, a2, X[ 3], 12, md_k5);
		md_Subround(md_J, a2, b2, c2, d2, e2, X[12],  6, md_k5);

		t = a1; a1 = a2; a2 = t;

		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 7],  7, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[ 4],  6, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[13],  8, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[ 1], 13, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[10], 11, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 6],  9, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[15],  7, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[ 3], 15, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[12],  7, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[ 0], 12, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 9], 15, md_k1);
		md_Subround(md_G, d1, e1, a1, b1, c1, X[ 5],  9, md_k1);
		md_Subround(md_G, c1, d1, e1, a1, b1, X[ 2], 11, md_k1);
		md_Subround(md_G, b1, c1, d1, e1, a1, X[14],  7, md_k1);
		md_Subround(md_G, a1, b1, c1, d1, e1, X[11], 13, md_k1);
		md_Subround(md_G, e1, a1, b1, c1, d1, X[ 8], 12, md_k1);

		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 6],  9, md_k6); 
		md_Subround(md_I, d2, e2, a2, b2, c2, X[11], 13, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[ 3], 15, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[ 7],  7, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[ 0], 12, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[13],  8, md_k6);
		md_Subround(md_I, d2, e2, a2, b2, c2, X[ 5],  9, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[10], 11, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[14],  7, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[15],  7, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 8], 12, md_k6);
		md_Subround(md_I, d2, e2, a2, b2, c2, X[12],  7, md_k6);
		md_Subround(md_I, c2, d2, e2, a2, b2, X[ 4],  6, md_k6);
		md_Subround(md_I, b2, c2, d2, e2, a2, X[ 9], 15, md_k6);
		md_Subround(md_I, a2, b2, c2, d2, e2, X[ 1], 13, md_k6);
		md_Subround(md_I, e2, a2, b2, c2, d2, X[ 2], 11, md_k6);

		t = b1; b1 = b2; b2 = t;

		md_Subround(md_H, d1, e1, a1, b1, c1, X[ 3], 11, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[10], 13, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[14],  6, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[ 4],  7, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 9], 14, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[15],  9, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[ 8], 13, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[ 1], 15, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[ 2], 14, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 7],  8, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[ 0], 13, md_k2);
		md_Subround(md_H, c1, d1, e1, a1, b1, X[ 6],  6, md_k2);
		md_Subround(md_H, b1, c1, d1, e1, a1, X[13],  5, md_k2);
		md_Subround(md_H, a1, b1, c1, d1, e1, X[11], 12, md_k2);
		md_Subround(md_H, e1, a1, b1, c1, d1, X[ 5],  7, md_k2);
		md_Subround(md_H, d1, e1, a1, b1, c1, X[12],  5, md_k2);

		md_Subround(md_H, d2, e2, a2, b2, c2, X[15],  9, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 5],  7, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[ 1], 15, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[ 3], 11, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 7],  8, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[14],  6, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 6],  6, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[ 9], 14, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[11], 12, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 8], 13, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[12],  5, md_k7);
		md_Subround(md_H, c2, d2, e2, a2, b2, X[ 2], 14, md_k7);
		md_Subround(md_H, b2, c2, d2, e2, a2, X[10], 13, md_k7);
		md_Subround(md_H, a2, b2, c2, d2, e2, X[ 0], 13, md_k7);
		md_Subround(md_H, e2, a2, b2, c2, d2, X[ 4],  7, md_k7);
		md_Subround(md_H, d2, e2, a2, b2, c2, X[13],  5, md_k7);

		t = c1; c1 = c2; c2 = t;

		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 1], 11, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[ 9], 12, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[11], 14, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[10], 15, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 0], 14, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 8], 15, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[12],  9, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[ 4],  8, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[13],  9, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 3], 14, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 7],  5, md_k3);
		md_Subround(md_I, b1, c1, d1, e1, a1, X[15],  6, md_k3);
		md_Subround(md_I, a1, b1, c1, d1, e1, X[14],  8, md_k3);
		md_Subround(md_I, e1, a1, b1, c1, d1, X[ 5],  6, md_k3);
		md_Subround(md_I, d1, e1, a1, b1, c1, X[ 6],  5, md_k3);
		md_Subround(md_I, c1, d1, e1, a1, b1, X[ 2], 12, md_k3);

		md_Subround(md_G, c2, d2, e2, a2, b2, X[ 8], 15, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[ 6],  5, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 4],  8, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 1], 11, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[ 3], 14, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[11], 14, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[15],  6, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 0], 14, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 5],  6, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[12],  9, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[ 2], 12, md_k8);
		md_Subround(md_G, b2, c2, d2, e2, a2, X[13],  9, md_k8);
		md_Subround(md_G, a2, b2, c2, d2, e2, X[ 9], 12, md_k8);
		md_Subround(md_G, e2, a2, b2, c2, d2, X[ 7],  5, md_k8);
		md_Subround(md_G, d2, e2, a2, b2, c2, X[10], 15, md_k8);
		md_Subround(md_G, c2, d2, e2, a2, b2, X[14],  8, md_k8);

		t = d1; d1 = d2; d2 = t;

		md_Subround(md_J, b1, c1, d1, e1, a1, X[ 4],  9, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 0], 15, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[ 5],  5, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[ 9], 11, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[ 7],  6, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[12],  8, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 2], 13, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[10], 12, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[14],  5, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[ 1], 12, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[ 3], 13, md_k4);
		md_Subround(md_J, a1, b1, c1, d1, e1, X[ 8], 14, md_k4);
		md_Subround(md_J, e1, a1, b1, c1, d1, X[11], 11, md_k4);
		md_Subround(md_J, d1, e1, a1, b1, c1, X[ 6],  8, md_k4);
		md_Subround(md_J, c1, d1, e1, a1, b1, X[15],  5, md_k4);
		md_Subround(md_J, b1, c1, d1, e1, a1, X[13],  6, md_k4);

		md_Subround(md_F, b2, c2, d2, e2, a2, X[12],  8, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[15],  5, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[10], 12, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 4],  9, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 1], 12, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[ 5],  5, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[ 8], 14, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[ 7],  6, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 6],  8, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 2], 13, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[13],  6, md_k9);
		md_Subround(md_F, a2, b2, c2, d2, e2, X[14],  5, md_k9);
		md_Subround(md_F, e2, a2, b2, c2, d2, X[ 0], 15, md_k9);
		md_Subround(md_F, d2, e2, a2, b2, c2, X[ 3], 13, md_k9);
		md_Subround(md_F, c2, d2, e2, a2, b2, X[ 9], 11, md_k9);
		md_Subround(md_F, b2, c2, d2, e2, a2, X[11], 11, md_k9);

		t = e1; e1 = e2; e2 = t;

		MDbuf[0] += a1;
		MDbuf[1] += b1;
		MDbuf[2] += c1;
		MDbuf[3] += d1;
		MDbuf[4] += e1;
		MDbuf[5] += a2;
		MDbuf[6] += b2;
		MDbuf[7] += c2;
		MDbuf[8] += d2;
		MDbuf[9] += e2;
	}

private:
	UI32 MDbuf[10];		//保存有ripemd输出值
	UI32 X[16];			//保存有每步变换时对输入的初始化转化值
};
//...end ripeMd320
#endif

