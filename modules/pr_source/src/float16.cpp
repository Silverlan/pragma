#include "float16.h"

void float16::Init() {m_storage.rawWord = 0;}
uint16_t float16::GetBits() const {return m_storage.rawWord;}
float float16::GetFloat() const {return Convert16bitFloatTo32bits(m_storage.rawWord);}
void float16::SetFloat(float in) {m_storage.rawWord = ConvertFloatTo16bits(in);}
bool float16::IsInfinity() const {return m_storage.bits.biased_exponent == 31 && m_storage.bits.mantissa == 0;}
bool float16::IsNaN() const {return m_storage.bits.biased_exponent == 31 && m_storage.bits.mantissa != 0;}

bool float16::operator==(const float16 other) const {return m_storage.rawWord == other.m_storage.rawWord;}
bool float16::operator!=(const float16 other) const {return m_storage.rawWord != other.m_storage.rawWord;}

bool float16::IsNaN(float16bits in) {return in.bits.biased_exponent == 31 && in.bits.mantissa != 0;}
bool float16::IsInfinity(float16bits in) {return in.bits.biased_exponent == 31 && in.bits.mantissa == 0;}

uint16_t float16::ConvertFloatTo16bits(float input)
{
	if(input > maxfloat16bits)
		input = maxfloat16bits;
	else if (input < -maxfloat16bits)
		input = -maxfloat16bits;

	float16bits output;
	float32bits inFloat;

	inFloat.rawFloat = input;
	output.bits.sign = inFloat.bits.sign;

	if((inFloat.bits.biased_exponent == 0) && (inFloat.bits.mantissa == 0))
	{
		// zero
		output.bits.mantissa = 0;
		output.bits.biased_exponent = 0;
	}
	else if((inFloat.bits.biased_exponent == 0) && (inFloat.bits.mantissa != 0))
	{
		// denorm -- denorm float maps to 0 half
		output.bits.mantissa = 0;
		output.bits.biased_exponent = 0;
	}
	else if((inFloat.bits.biased_exponent == 0xff) && (inFloat.bits.mantissa == 0))
	{
#if 0
		// infinity
		output.bits.mantissa = 0;
		output.bits.biased_exponent = 31;
#else
		// infinity maps to maxfloat
		output.bits.mantissa = 0x3ff;
		output.bits.biased_exponent = 0x1e;
#endif
	}
	else if((inFloat.bits.biased_exponent == 0xff) && (inFloat.bits.mantissa != 0))
	{
#if 0
		// NaN
		output.bits.mantissa = 1;
		output.bits.biased_exponent = 31;
#else
		// NaN maps to zero
		output.bits.mantissa = 0;
		output.bits.biased_exponent = 0;
#endif
	}
	else
	{
		// regular number
		int new_exp = inFloat.bits.biased_exponent -127;
		if(new_exp < -24)
		{ 
			// this maps to 0
			output.bits.mantissa = 0;
			output.bits.biased_exponent = 0;
		}

		if(new_exp < -14)
		{
			// this maps to a denorm
			output.bits.biased_exponent = 0;
			auto exp_val = static_cast<uint32_t>(-14 -( inFloat.bits.biased_exponent -float32bias));
			if(exp_val > 0 && exp_val < 11)
				output.bits.mantissa = (1 << (10 -exp_val)) +(inFloat.bits.mantissa >> (13 +exp_val));
		}
		else if(new_exp > 15)
		{
#if 0
			// map this value to infinity
			output.bits.mantissa = 0;
			output.bits.biased_exponent = 31;
#else
			// to big. . . maps to maxfloat
			output.bits.mantissa = 0x3ff;
			output.bits.biased_exponent = 0x1e;
#endif
		}
		else
		{
			output.bits.biased_exponent = new_exp +15;
			output.bits.mantissa = (inFloat.bits.mantissa >> 13);
		}
	}
	return output.rawWord;
}

float float16::Convert16bitFloatTo32bits(uint16_t input)
{
	float32bits output;
	float16bits inFloat;
	inFloat.rawWord = input;
	if(IsInfinity(inFloat))
		return maxfloat16bits *((inFloat.bits.sign == 1) ? -1.0f : 1.0f);
	if(IsNaN(inFloat))
		return 0.0;
	if(inFloat.bits.biased_exponent == 0 && inFloat.bits.mantissa != 0)
	{
		// denorm
		const float half_denorm = (1.0f /16'384.0f); // 2^-14
		float mantissa = ((float)(inFloat.bits.mantissa)) /1024.0f;
		float sgn = (inFloat.bits.sign) ? -1.0f : 1.0f;
		output.rawFloat = sgn *mantissa *half_denorm;
	}
	else
	{
		// regular number
		output.bits.mantissa = inFloat.bits.mantissa << (23 -10);
		output.bits.biased_exponent = (inFloat.bits.biased_exponent -float16bias +float32bias) *(inFloat.bits.biased_exponent != 0);
		output.bits.sign = inFloat.bits.sign;
	}
	return output.rawFloat;
}
