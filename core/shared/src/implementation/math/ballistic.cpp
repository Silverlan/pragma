// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :math.ballistic;

Vector3 pragma::math::calc_ballistic_position(const Vector3 &start, const Vector3 &vel, float gravity, float t) { return start + vel * t + 0.5f * Vector3 {0.f, -gravity, 0.f} * t * t; }

bool pragma::math::calc_ballistic_velocity(const Vector3 &start, const Vector3 &end, float angle, float gravity, Vector3 &vel)
{
	// Source: http://www.theappguruz.com/blog/hit-target-using-ballistic-trajectory
	auto dir = end - start;
	auto h = dir.y;

	auto dist = uvec::length(dir);
	auto a = angle;
	dir.y += dist * tan(a);
	uvec::normalize(&dir);

	auto ta = tan(a);
	if(ta == 0.f)
		return false;
	dist += h / ta;
	auto s = sin(2.0 * a);
	if(s == 0.f)
		return false;
	vel = dir * sqrtf(dist * gravity / s);
	return true;
}

float pragma::math::calc_ballistic_time_of_flight(const Vector3 &start, float launchAngle, float velocity, float gravity)
{
	// Source: https://en.wikipedia.org/wiki/Trajectory_of_a_projectile
	launchAngle = launchAngle;
	auto s = sin(launchAngle);
	auto y0 = start.y;
	return (velocity * s + sqrtf((velocity * s) * (velocity * s) + 2.f * gravity * y0)) / gravity;
}
float pragma::math::calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &vel, float gravity)
{
	auto l = uvec::length(vel);
	auto ang = uvec::to_angle(vel / l);
	return calc_ballistic_time_of_flight(start, -ang.p, l, gravity);
}
float pragma::math::calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &end, float launchAngle, float velocity, float gravity) { return calc_ballistic_time_of_flight(start - end, launchAngle, velocity, gravity); }
float pragma::math::calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &end, const Vector3 &vel, float gravity) { return calc_ballistic_time_of_flight(start - end, vel, gravity); }

float pragma::math::calc_ballistic_angle_of_reach(const Vector3 &start, float distance, float initialVelocity, float gravity)
{
	// Source: https://en.wikipedia.org/wiki/Trajectory_of_a_projectile
	return 0.5f * asin((gravity * distance) / (initialVelocity * initialVelocity));
}

/////////////////////////////////////

// Source: https://blog.forrestthewoods.com/solving-ballistic-trajectories-b0165523348c

// Calculate the maximum range that a ballistic projectile can be fired on given speed and gravity.
//
// speed (float): projectile velocity
// gravity (float): force of gravity, positive is down
// initial_height (float): distance above flat terrain
//
// return (float): maximum range
float pragma::math::calc_ballistic_range(float speed, float gravity, float initialHeight)
{
	// Handling these cases is up to your project's coding standards
	assert(speed > 0.f && gravity > 0.f && initialHeight >= 0.f);

	// Derivation
	//   (1) x = speed * time * cos O
	//   (2) y = initial_height + (speed * time * sin O) - (.5 * gravity*time*time)
	//   (3) via quadratic: t = (speed*sin O)/gravity + sqrt(speed*speed*sin O + 2*gravity*initial_height)/gravity    [ignore smaller root]
	//   (4) solution: range = x = (speed*cos O)/gravity * sqrt(speed*speed*sin O + 2*gravity*initial_height)    [plug t back into x=speed*time*cos O]
	auto angle = deg_to_rad(45.f); // no air resistence, so 45 degrees provides maximum range
	auto cos = math::cos(angle);
	auto sin = math::sin(angle);

	return (speed * cos / gravity) * (speed * sin + sqrtf(speed * speed * sin * sin + 2 * gravity * initialHeight));
}

// Solve firing angles for a ballistic projectile with speed and gravity to hit a fixed position.
//
// proj_pos (Vector3): point projectile will fire from
// proj_speed (float): scalar speed of projectile
// target (Vector3): point projectile is trying to hit
// gravity (float): force of gravity, positive down
//
// s0 (out Vector3): firing solution (low angle)
// s1 (out Vector3): firing solution (high angle)
//
// return (int): number of unique solutions found: 0, 1, or 2.
int32_t pragma::math::solve_ballistic_arc(const Vector3 &projPos, float projSpeed, const Vector3 &target, float gravity, std::array<Vector3, 2> &s)
{
	// Handling these cases is up to your project's coding standards
	assert(projPos != target && projSpeed > 0.f && gravity > 0.f);

	// C# requires out variables be set
	s[0] = uvec::PRM_ORIGIN;
	s[1] = uvec::PRM_ORIGIN;

	// Derivation
	//   (1) x = v*t*cos O
	//   (2) y = v*t*sin O - .5*g*t^2
	//
	//   (3) t = x/(cos O*v)                                        [solve t from (1)]
	//   (4) y = v*x*sin O/(cos O * v) - .5*g*x^2/(cos^2 O*v^2)     [plug t into y=...]
	//   (5) y = x*tan O - g*x^2/(2*v^2*cos^2 O)                    [reduce; cos/sin = tan]
	//   (6) y = x*tan O - (g*x^2/(2*v^2))*(1+tan^2 O)              [reduce; 1+tan O = 1/cos^2 O]
	//   (7) 0 = ((-g*x^2)/(2*v^2))*tan^2 O + x*tan O - (g*x^2)/(2*v^2) - y    [re-arrange]
	//   Quadratic! a*p^2 + b*p + c where p = tan O
	//
	//   (8) let gxv = -g*x*x/(2*v*v)
	//   (9) p = (-x +- sqrt(x*x - 4gxv*(gxv - y)))/2*gxv           [quadratic formula]
	//   (10) p = (v^2 +- sqrt(v^4 - g(g*x^2 + 2*y*v^2)))/gx        [multiply top/bottom by -2*v*v/x; move 4*v^4/x^2 into root]
	//   (11) O = atan(p)

	auto diff = target - projPos;
	auto diffXZ = Vector3(diff.x, 0.f, diff.z);
	auto groundDist = uvec::length(diffXZ);

	auto speed2 = projSpeed * projSpeed;
	auto speed4 = projSpeed * projSpeed * projSpeed * projSpeed;
	auto y = diff.y;
	auto x = groundDist;
	auto gx = gravity * x;

	auto root = speed4 - gravity * (gravity * x * x + 2 * y * speed2);

	// No solution
	if(root < 0)
		return 0;

	root = sqrtf(root);

	auto lowAng = atan2(speed2 - root, gx);
	auto highAng = atan2(speed2 + root, gx);
	int32_t numSolutions = (lowAng != highAng) ? 2 : 1;

	auto groundDir = uvec::get_normal(diffXZ);
	s[0] = groundDir * static_cast<float>(cos(lowAng) * projSpeed) + uvec::PRM_UP * static_cast<float>(sin(lowAng) * projSpeed);
	if(numSolutions > 1)
		s[1] = groundDir * static_cast<float>(cos(highAng) * projSpeed) + uvec::PRM_UP * static_cast<float>(sin(highAng) * projSpeed);
	return numSolutions;
}

// Solve firing angles for a ballistic projectile with speed and gravity to hit a target moving with constant, linear velocity.
//
// proj_pos (Vector3): point projectile will fire from
// proj_speed (float): scalar speed of projectile
// target (Vector3): point projectile is trying to hit
// target_velocity (Vector3): velocity of target
// gravity (float): force of gravity, positive down
//
// s0 (out Vector3): firing solution (fastest time impact)
// s1 (out Vector3): firing solution (next impact)
// s2 (out Vector3): firing solution (next impact)
// s3 (out Vector3): firing solution (next impact)
//
// return (int): number of unique solutions found: 0, 1, 2, 3, or 4.
int32_t pragma::math::solve_ballistic_arc(const Vector3 &projPos, float projSpeed, const Vector3 &targetPos, const Vector3 &targetVelocity, float gravity, std::array<Vector3, 2> &s)
{
	// Initialize output parameters
	s[0] = uvec::PRM_ORIGIN;
	s[1] = uvec::PRM_ORIGIN;

	// Derivation
	//
	//  For full derivation see: blog.forrestthewoods.com
	//  Here is an abbreviated version.
	//
	//  Four equations, four unknowns (solution.x, solution.y, solution.z, time):
	//
	//  (1) proj_pos.x + solution.x*time = target_pos.x + target_vel.x*time
	//  (2) proj_pos.y + solution.y*time + .5*G*t = target_pos.y + target_vel.y*time
	//  (3) proj_pos.z + solution.z*time = target_pos.z + target_vel.z*time
	//  (4) proj_speed^2 = solution.x^2 + solution.y^2 + solution.z^2
	//
	//  (5) Solve for solution.x and solution.z in equations (1) and (3)
	//  (6) Square solution.x and solution.z from (5)
	//  (7) Solve solution.y^2 by plugging (6) into (4)
	//  (8) Solve solution.y by rearranging (2)
	//  (9) Square (8)
	//  (10) Set (8) = (7). All solution.xyz terms should be gone. Only time remains.
	//  (11) Rearrange 10. It will be of the form a*^4 + b*t^3 + c*t^2 + d*t * e. This is a quartic.
	//  (12) Solve the quartic using SolveQuartic.
	//  (13) If there are no positive, real roots there is no solution.
	//  (14) Each positive, real root is one valid solution
	//  (15) Plug each time value into (1) (2) and (3) to calculate solution.xyz
	//  (16) The end.

	auto G = gravity;

	auto A = projPos.x;
	auto B = projPos.y;
	auto C = projPos.z;
	auto M = targetPos.x;
	auto N = targetPos.y;
	auto O = targetPos.z;
	auto P = targetVelocity.x;
	auto Q = targetVelocity.y;
	auto R = targetVelocity.z;
	auto S = projSpeed;

	auto H = M - A;
	auto J = O - C;
	auto K = N - B;
	auto L = -.5f * G;

	// Quartic Coeffecients
	std::array<double, 5> c = {L * L, 2 * Q * L, Q * Q + 2 * K * L - S * S + P * P + R * R, 2 * K * Q + 2 * H * P + 2 * J * R, K * K + H * H + J * J};

	// Solve quartic
	std::array<double, 4> times;
	auto numTimes = solve_quartic(c, times);

	// Sort so faster collision is found first
	std::sort(times.begin(), times.end());

	// Plug quartic solutions into base equations
	// There should never be more than 2 positive, real roots.
	std::array<Vector3, 2> solutions;
	int32_t numSolutions = 0;

	for(auto i = decltype(numSolutions) {0}; i < 2; ++i) {
		auto t = times[i];
		if(t <= 0)
			continue;

		solutions[numSolutions].x = static_cast<float>((H + P * t) / t);
		solutions[numSolutions].y = static_cast<float>((K + Q * t - L * t * t) / t);
		solutions[numSolutions].z = static_cast<float>((J + R * t) / t);
		++numSolutions;
	}

	// Write out solutions
	if(numSolutions > 0)
		s[0] = solutions[0];
	if(numSolutions > 1)
		s[1] = solutions[1];
	return numSolutions;
}

// Solve the firing arc with a fixed lateral speed. Vertical speed and gravity varies.
// This enables a visually pleasing arc.
//
// proj_pos (Vector3): point projectile will fire from
// lateral_speed (float): scalar speed of projectile along XZ plane
// target_pos (Vector3): point projectile is trying to hit
// max_height (float): height above Max(proj_pos, impact_pos) for projectile to peak at
//
// fire_velocity (out Vector3): firing velocity
// gravity (out float): gravity necessary to projectile to hit precisely max_height
//
// return (bool): true if a valid solution was found
bool pragma::math::solve_ballistic_arc_lateral(const Vector3 &projPos, float lateralSpeed, const Vector3 &targetPos, float maxHeight, Vector3 &fireVelocity, float &gravity)
{
	// Handling these cases is up to your project's coding standards
	assert(projPos != targetPos && lateralSpeed > 0.f && maxHeight > projPos.y);

	fireVelocity = uvec::PRM_ORIGIN;
	gravity = std::numeric_limits<float>::quiet_NaN();

	auto diff = targetPos - projPos;
	auto diffXZ = Vector3(diff.x, 0.f, diff.z);
	auto lateralDist = uvec::length(diffXZ);

	if(lateralDist == 0.f)
		return false;

	auto time = lateralDist / lateralSpeed;

	fireVelocity = uvec::get_normal(diffXZ) * lateralSpeed;

	// System of equations. Hit max_height at t=.5*time. Hit target at t=time.
	//
	// peak = y0 + vertical_speed*halfTime + .5*gravity*halfTime^2
	// end = y0 + vertical_speed*time + .5*gravity*time^s
	// Wolfram Alpha: solve b = a + .5*v*t + .5*g*(.5*t)^2, c = a + vt + .5*g*t^2 for g, v
	auto a = projPos.y;   // initial
	auto b = maxHeight;   // peak
	auto c = targetPos.y; // final

	gravity = -4 * (a - 2 * b + c) / (time * time);
	fireVelocity.y = -(3 * a - 4 * b + c) / time;
	return true;
}

// Solve the firing arc with a fixed lateral speed. Vertical speed and gravity varies.
// This enables a visually pleasing arc.
//
// proj_pos (Vector3): point projectile will fire from
// lateral_speed (float): scalar speed of projectile along XZ plane
// target_pos (Vector3): point projectile is trying to hit
// max_height (float): height above Max(proj_pos, impact_pos) for projectile to peak at
//
// fire_velocity (out Vector3): firing velocity
// gravity (out float): gravity necessary to projectile to hit precisely max_height
// impact_point (out Vector3): point where moving target will be hit
//
// return (bool): true if a valid solution was found
bool pragma::math::solve_ballistic_arc_lateral(const Vector3 &projPos, float lateralSpeed, const Vector3 &target, const Vector3 &targetVelocity, float maxHeightOffset, Vector3 &fireVelocity, float &gravity, Vector3 &impactPoint)
{
	// Handling these cases is up to your project's coding standards
	assert(projPos != target && lateralSpeed > 0.f);

	// Initialize output variables
	fireVelocity = uvec::PRM_ORIGIN;
	gravity = 0.f;
	impactPoint = uvec::PRM_ORIGIN;

	// Ground plane terms
	auto targetVelXZ = Vector3(targetVelocity.x, 0.f, targetVelocity.z);
	auto diffXZ = target - projPos;
	diffXZ.y = 0.f;

	// Derivation
	//   (1) Base formula: |P + V*t| = S*t
	//   (2) Substitute variables: |diffXZ + targetVelXZ*t| = S*t
	//   (3) Square both sides: Dot(diffXZ,diffXZ) + 2*Dot(diffXZ, targetVelXZ)*t + Dot(targetVelXZ, targetVelXZ)*t^2 = S^2 * t^2
	//   (4) Quadratic: (Dot(targetVelXZ,targetVelXZ) - S^2)t^2 + (2*Dot(diffXZ, targetVelXZ))*t + Dot(diffXZ, diffXZ) = 0
	std::array<double, 3> c = {uvec::dot(targetVelXZ, targetVelXZ) - lateralSpeed * lateralSpeed, 2.f * uvec::dot(diffXZ, targetVelXZ), uvec::dot(diffXZ, diffXZ)};

	std::array<double, 2> t;
	auto n = solve_quadric(c, t);

	// pick smallest, positive time
	auto valid0 = (n > 0) && t[0] > 0;
	auto valid1 = (n > 1) && t[1] > 0;

	float tr;
	if(!valid0 && !valid1)
		return false;
	else if(valid0 && valid1)
		tr = min(static_cast<float>(t[0]), static_cast<float>(t[1]));
	else
		tr = valid0 ? static_cast<float>(t[0]) : static_cast<float>(t[1]);

	// Calculate impact point
	impactPoint = target + (targetVelocity * tr);

	// Calculate fire velocity along XZ plane
	auto dir = impactPoint - projPos;
	dir.y = 0.f;
	uvec::normalize(&dir);
	fireVelocity = dir * lateralSpeed;

	// Solve system of equations. Hit max_height at t=.5*time. Hit target at t=time.
	//
	// peak = y0 + vertical_speed*halfTime + .5*gravity*halfTime^2
	// end = y0 + vertical_speed*time + .5*gravity*time^s
	// Wolfram Alpha: solve b = a + .5*v*t + .5*g*(.5*t)^2, c = a + vt + .5*g*t^2 for g, v
	auto a = projPos.y;                                              // initial
	auto b = max(projPos.y, impactPoint.y) + maxHeightOffset; // peak
	auto cr = impactPoint.y;                                         // final

	gravity = -4 * (a - 2 * b + cr) / (tr * tr);
	fireVelocity.y = -(3 * a - 4 * b + cr) / tr;
	return true;
}
