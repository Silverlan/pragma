#if 0
#ifndef __DMX_ANIMATION_HPP__
#define __DMX_ANIMATION_HPP__

namespace dmx
{
	class DmeModelRoot
	{
		DmeModel	skeleton;
	
		DmeAnimationList	animationList;
	};

	class DmeAnimationList
	{
		DmeChannelsClip animations[];
	};

	class DmeChannelsClip
	{
		DmeTimeFrame timeFrame;
		Colour color;		// SFM only
		CUtlString text;	// SFM only
		bool mute;			// SFM only
		int frameRate;		// typically 30

		DmeTrackGroup trackGroups[];  // SFM only
		DmeChannel channels[];	// two for each bone: position and rotation
	};

	class DmeTimeFrame
	{
		DmeTime_t start;	// no apparent effect, use offset
		DmeTime_t duration;	// length in seconds...framerate is NOT adjusted
		DmeTime_t offset;	// remove this many seconds from the start (can be negative)
		float scale; // frameRate multiplier
	};

	class DmeChannel
	{
		// this format is shared with Source Filmmaker, so has support for animating generic properties.
		// Studiomdl only cares about bones though.
		CDmxElement	fromElement; // TODO: what should this be?
		CUtlString		fromAttribute;
		int				fromIndex;
	
		CDmxElement	toElement; // ordinarily a DmeTransform used by the target bone
		CUtlString		toAttribute;
		int				toIndex;
	
		int		mode; // Recording mode for channel - unused by studiomdl.

		// One of:
		DmeQuaternionLog	log[];
		DmeVector3Log		log[];
		// etc
	};

	class DmeQuaternionLog // also DmeVector3Log etc.
	{
		DmeQuaternionLogLayer layers[];

		CDmxElement	curveinfo;
		bool			usedefaultvalue;
		Quaternion		defaultvalue;
	};

	class DmeQuaternionLogLayer // also DmeVector3LogLayer etc.
	{
		// only frames where the bone moves need to be given
		// unlike SMD, sparse keyframes are supported
		DmeTime_t	times[];
		int			curvetypes[]; // keyframe interp in SFM
		Quaternion	values[];
	};
};

#endif
#endif