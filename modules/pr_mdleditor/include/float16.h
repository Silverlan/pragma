#ifndef __FLOAT16_H__
#define __FLOAT16_H__

const int float32bias = 127;
const int float16bias = 15;

const float maxfloat16bits = 65504.0f;

class float16
{
public:
	void Init();
	uint16_t GetBits() const;
	float GetFloat() const;
	void SetFloat(float in);
	bool IsInfinity() const;
	bool IsNaN() const;

	bool operator==(const float16 other) const;
	bool operator!=(const float16 other) const;
protected:
	union float32bits
	{
		float rawFloat;
		struct 
		{
			uint32_t mantissa : 23;
			uint32_t biased_exponent : 8;
			uint32_t sign : 1;
		} bits;
	};
	union float16bits
	{
		uint16_t rawWord;
		struct
		{
			uint16_t mantissa : 10;
			uint16_t biased_exponent : 5;
			uint16_t sign : 1;
		} bits;
	};

	static bool IsNaN(float16bits in);
	static bool IsInfinity(float16bits in);
	// 0x0001 - 0x03ff
	float16bits m_storage;
public:
	static uint16_t ConvertFloatTo16bits(float input);
	static float Convert16bitFloatTo32bits(uint16_t input);
};

#endif
