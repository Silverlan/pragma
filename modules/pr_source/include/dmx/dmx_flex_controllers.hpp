#if 0
#ifndef __DMX_FLEX_CONTROLLERS_HPP__
#define __DMX_FLEX_CONTROLLERS_HPP__

namespace dmx
{
	class DmeCombinationOperator // flex controller global settings
	{
		DmeCombinationInputControl		controls[];
		Vector							controlValues[];		// rest position for L/R values
		Vector							controlValuesLagged[];	// lerp factor for changes to the values (if usesLaggedValues == true)
		bool							usesLaggedValues;       // value changes are not instant, but "lagged" (lerp between values - enabled by default in SMD)
		DmeCombinationDominationRule	dominators[]; // list of domination rules to use
		DmeMesh							targets[]; // mesh with shapes on, or DmeFlexRules in some old DMX files
	};

	class DmeCombinationInputControl // a flex controller
	{
		CUtlString	rawControlNames[];	// which controls are being wrapped
		bool		stereo;				// equivalent to QC 'split'
		bool		eyelid;				// flags as an eyelid used by AI for blinking
		float		wrinkleScales[];	// records the scale used to generate wrinkle data; not read by studiomdl
		float		flexMin;
		float		flexMax;
	};

	class DmeCombinationDominationRule // Disables certain shapes (NOT controllers) when others are active
	{
		CUtlString	dominators[];
		CUtlString	supressed[];
	};

	class DmeFlexRules // shape key pre-processing. Insert at DmeCombinationOperator::targets.
	{
		DmeFlexRule	deltaStates[]; // mixed type
		Vector2								deltaStateWeights[];
		DmeMesh								target; // mesh with the shapes on
	};

	// In flex rules, the element name must match the name of a DmeVertexDeltaData element on the target DmeMesh.
	// It does NOT specify values of controllers.
	class DmeFlexRule
	{
		float		result;
	};
	class DmeFlexRuleExpression : DmeFlexRule // Seems to be replaced with "DmeFlexRule" in older versions of DMX?
	{
		float		result;
		CUtlString	expression; // +-/() with min, max & sqrt. Flex controller names can be included too, as long as their names don't have spaces!
								// L/R split controllers either have the left_ or right_ prefix, or a L or R suffix.
	};
	class DmeFlexRulePassThrough : DmeFlexRule // No expression required, shapes are controlled like normal
	{
		float		result;
	};
};

#endif
#endif