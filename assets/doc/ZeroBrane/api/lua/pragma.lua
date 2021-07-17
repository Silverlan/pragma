return {
	["ents"] = {
		type = "lib",
		description = [[]],
		childs = {
			["create"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["iterator"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["DamageableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["TakeDamage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["tf2"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["FlammableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["Ignite"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIgnitableProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOnFireProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsIgnitable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Extinguish"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetIgnitable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsOnFire"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["RenderTargetComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["CharacterComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["RemoveAmmo"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetOrientation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsMoving"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUpDirection"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsDead"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrozenProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAlive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFrozen"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasWeapon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFrozen"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHitboxPhysicsObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActiveWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LocalOrientationToWorld"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetJumpPowerProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Kill"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveWeapon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOrientationAxes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMoveController"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetOrientationAxesRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRelativeVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShootPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAmmoCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSlopeLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalOrientationViewAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStepOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTurnSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["NormalizeViewRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GiveWeapon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetViewRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Ragdolize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUpDirectionProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSlopeLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStepOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveWeapons"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTurnSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUpDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetViewAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetViewRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWeapons"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAmmoCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWeaponCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveController"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAimRayData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSlopeLimitProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FootStep"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNeckControllers"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetNeckYawController"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNeckPitchController"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalOrientationAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalOrientationRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DropActiveWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Attack4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalOrientationViewRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WorldToLocalOrientation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddAmmo"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetStepOffsetProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetJumpPower"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetJumpPower"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DropWeapon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Jump"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DeployWeapon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HolsterWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SelectNextWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SelectPreviousWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PrimaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SecondaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TertiaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReloadWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNoTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNoTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFaction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetGodMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGodMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFaction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["GamemodeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetIdentifier"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAuthor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetGamemodeVersion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["FuncSoftPhysicsComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["IteratorFilterClass"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["FilterNameComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["ShouldPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["CompositeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["AddEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearEntities"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEntities"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetRootGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CompositeGroup"] = {
						type = "class",
						description = [[]],
						childs = {
							["RemoveEntity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearEntities"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["AddChildGroup"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetEntities"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetChildGroups"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["AddEntity"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							}
						}
					},

				}
			},
			["SoundComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetSoundSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRolloffFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMinGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetInnerConeAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOuterConeAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTimeOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTimeOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetReferenceDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRelativeToListener"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPlayOnSpawn"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsPaused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLooping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoundTypes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Play"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Stop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPlaying"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Pause"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPitch"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["IteratorFilterUuid"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["IteratorFilterCone"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["TransformComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LocalToWorld"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastMoveTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPitch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEyePos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEyeOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEyeOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WorldToLocal"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetYaw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRoll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPitch"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetYaw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRoll"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxAxisScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAbsMaxAxisScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DebugCylinderComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LogicRelayComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointConstraintBallSocketComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["EnvSoundProbeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PlayerSpawnComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["EnvTimeScaleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoundDspEqualizerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LightPointComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["RenderComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetRenderBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRenderClipPlane"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCastShadows"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsExemptFromOcclusionCulling"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCastShadows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetReceiveShadows"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDepthBias"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcRayIntersection"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetTransformationMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsReceivingShadows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAbsoluteRenderSphereBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRenderBufferDirty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLODMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDepthPassEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDepthPassEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderClipPlane"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearRenderClipPlane"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalRenderSphereBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearDepthBias"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocalRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBoneBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLodRenderMeshes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetExemptFromOcclusionCulling"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearRenderOffsetTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShouldDrawShadow"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClearBuffers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShouldDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ShouldCastShadows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetRenderPose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderOffsetTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAbsoluteRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderModeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRenderMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthBias"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetRenderOffsetTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DebugConeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["IteratorFilterBox"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["SoundDspComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ParentComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PropDynamicComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["TimeScaleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetTimeScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTimeScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEffectiveTimeScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ParticleSystemComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetColorFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Stop"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParticleBufferIndexFromParticleIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetControlPointPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRadius"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBloomColorFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PauseEmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEmissionRate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Die"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRadius"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetColorFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSpriteSheetAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetExtent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationSpriteSheetBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsDying"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAutoSimulate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCastShadows"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ResumeEmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCastShadows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsBloomEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRenderMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRenderMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNodePosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddOperator"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRenderParticleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Start"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveOperatorByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLifeTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetControlPointEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Render"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetControlPointRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetInitialColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetControlPointEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InitializeFromParticleSystemDefinition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ShouldAutoSimulate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Simulate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsAnimated"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSortParticles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSoftParticles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParticleAnimationBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetControlPointPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetParticleBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNextParticleEmissionCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSimulationTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEmissionRate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsEmissionPaused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetInitialColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParticleIndexFromParticleBufferIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsActiveOrPaused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStartTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftParticles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSortParticles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBloomColorFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEffectiveBloomColorFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParticleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParticle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParticles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxParticleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsActive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEffectiveAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddInitializer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddRenderer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RemoveInitializer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveOperator"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveRenderer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveInitializerByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNodeCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveRendererByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInitializers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOperators"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindInitializer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindOperator"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsStatic"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindInitializerByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindOperatorByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindRendererByType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNodeTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRemoveOnComplete"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetExtent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOrientationType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsContinuous"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetContinuous"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRemoveOnComplete"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrevControlPointPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetControlPointPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GenerateModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOrientationType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaterial"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNodeTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["FindRenderer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ParticleRenderer"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {

						}
					},
					["BaseOperator"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleOperator ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleDestroyed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["OnParticleSystemStarted"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleSystemStopped"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleCreated"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Simulate"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["Initialize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["ParticleModifier"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetKeyValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetPriority"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetKeyValues"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetType"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetKeyValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetPriority"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetParticleSystem"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["BaseInitializer"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleInitializer ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleDestroyed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["OnParticleSystemStopped"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleSystemStarted"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleCreated"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Initialize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["ParticleInitializer"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {

						}
					},
					["Particle"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetField"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetRadius"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetPreviousPosition"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetOrigin"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAlpha"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetField"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetLife"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPreviousPosition"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRotationYaw"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetAlive"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVelocity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetRotationYaw"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetColor"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["CalcRandomFloatExp"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function, : Function)",
								returns = "()"
							},
							["GetTimeAlive"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetCameraDistance"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["CalcRandomFloat"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetInitialRotation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetInitialLife"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTimeAlive"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTimeCreated"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetDeathTime"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetRotation"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetPosition"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetLength"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPosition"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Resurrect"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCameraDistance"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetAnimationFrameOffset"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["IsAlive"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Die"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetRadius"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Reset"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLength"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetExtent"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetLifeSpan"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetInitialAnimationFrameOffset"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetWorldRotation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAnimationFrameOffset"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetAngularVelocity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetAlpha"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetInitialLength"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAngularVelocity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTimeCreated"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLife"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ShouldDraw"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["CalcRandomInt"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetVelocity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetRotation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSeed"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsDying"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetOrigin"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetWorldRotation"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSequence"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetInitialRadius"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetInitialColor"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetInitialAlpha"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSequence"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["ParticleOperator"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {
							["CalcStrength"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["BaseRenderer"] = {
						type = "class",
						inherits = "ents.ParticleSystemComponent.ParticleRenderer ents.ParticleSystemComponent.ParticleModifier",
						description = [[]],
						childs = {
							["OnParticleSystemStarted"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleSystemStopped"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["OnParticleCreated"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetShader"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["OnParticleDestroyed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Render"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function, : Function)",
								returns = "()"
							},
							["GetShader"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Initialize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},

				}
			},
			["BotComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["AIComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["IsAIEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMemoryDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Forget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHearingStrength"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CanSee"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInViewCone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHearingStrength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CanHear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxViewDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLookTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMemoryFragmentCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrimaryTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlayActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearMoveSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasPrimaryTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNPCState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNPCState"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsMoving"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSquad"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAIEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EnableAI"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DisableAI"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MoveTo"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsControllable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetControllable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMemory"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMemoryDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["StartControl"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LockAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EndControl"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsControlled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["StopMoving"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetController"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAnimationLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Memorize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsEnemy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["TurnStep"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDistanceToMoveTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasReachedDestination"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveActivity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetControllerActionInput"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TriggerScheduleInterrupt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PlayAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMaxViewDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentSchedule"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDisposition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearRelationship"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSquadName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLookTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxViewAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxViewAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInMemory"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMoveSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearLookTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSquad"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["StartSchedule"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CancelSchedule"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRelationship"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearMemory"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AnimationInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetFaceTarget"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFacePrimaryTarget"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearFaceTarget"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["flags"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["playAsSchedule"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["IOComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["Input"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["StoreOutput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["FireOutput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["WorldComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetBSPTree"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["BSPTree"] = {
						type = "class",
						description = [[]],
						childs = {
							["FindLeafNode"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["FindLeafNodesInAABB"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["IsClusterVisible"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["IsValid"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetRootNode"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetClusterVisibility"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetNodes"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetClusterCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Node"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetBounds"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetInternalNodeFaceCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetCluster"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetInternalNodeFirstFaceIndex"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetVisibleLeafAreaBounds"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetInternalNodePlane"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetChildren"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetIndex"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["IsLeaf"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									}
								}
							},

						}
					},

				}
			},
			["SceneComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["InitializeRenderTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRenderQueue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetActiveCamera"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderParticleSystems"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RenderPrepass"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParticleSystemColorFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParticleSystemColorFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetActiveCamera"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOcclusionCullingMethod"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["BeginDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateBuffers"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWorldEnvironment"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWorldEnvironment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCameraDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewCameraDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Render"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetDebugMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["BuildRenderQueue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearWorldEnvironment"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRenderer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSceneIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CreateInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["sampleCount"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["FlashlightComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointConstraintSliderComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["RadiusComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetRadiusProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRadius"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRadius"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["FuncPortalComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ToggleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["TurnOn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TurnOff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Toggle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTurnedOn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTurnedOff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTurnedOn"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTurnedOnProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["LightDirectionalComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetAmbientColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAmbientColorProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAmbientColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["GravityComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetGravityScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetGravityOverride"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasGravityForceOverride"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasGravityDirectionOverride"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcBallisticVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetGravityDirection"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGravity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGravityForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["WheelComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["AnimatedComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetLayeredActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLayeredAnimationFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetGlobalBonePos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLayeredAnimationFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnimationObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSkeletonUpdateCallbacksEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PlayActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGlobalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PlayAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActivity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLayeredAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["PlayLayeredAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetRootPoseBoneId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBonePos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["PlayLayeredActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["BindAnimationEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["StopLayeredAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLayeredAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearAnimationEvents"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ApplyLayeredAnimations"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetBoneMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneBindPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCycle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBonePos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneRot"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGlobalBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetCycle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBoneRenderMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPlaybackRateProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBoneScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLocalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetBoneRenderMatrices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocalBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRootPoseBoneId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetLocalBonePos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddAnimationEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetBlendController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocalBoneRot"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBindPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetGlobalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["InjectAnimationEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetVertexPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetGlobalBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBaseAnimationFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetGlobalBoneRot"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEffectiveBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEffectiveBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBlendController"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetAnimatedRootPoseTransformEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBoneRot"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["UpdateEffectiveBoneTransforms"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBindPose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAnimatedRootPoseTransformEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddRootPoseBone"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetBaseAnimationFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBoneAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBoneRenderMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalVertexPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AreSkeletonUpdateCallbacksEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["TriggerTeleportComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SpriteComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["StopAndRemoveEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ObservableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetLocalCameraOrigin"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLocalCameraOrigin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocalCameraOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLocalCameraOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCameraOffsetProperty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCameraData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCameraEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCameraEnabledProperty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CameraData"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetAngleLimits"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetAngleLimits"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearAngleLimits"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["enabled"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["localOrigin"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rotateWithObservee"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["LightSpotVolComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoundEmitterComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["CreateSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["EmitSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["StopSounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["EntityComponent"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetNextTick"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddEventCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RegisterNetEvent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetComponentName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Load"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InjectEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SendNetEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["BroadcastEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InvokeEventCallbacks"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetComponentId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTickPolicy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTickPolicy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetNextTick"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["SoundDspChorusComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["WeaponComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetMaxPrimaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrimaryAmmoTypeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSecondaryClip"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Attack4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RefillPrimaryClip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RefillSecondaryClip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemovePrimaryClip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveSecondaryClip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PrimaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasPrimaryAmmo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPrimaryAmmoType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSecondaryAmmoType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SecondaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrimaryClipSizeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TertiaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNextPrimaryAttack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNextSecondaryAttack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNextAttack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAutomaticPrimary"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSecondaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAutomaticSecondary"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPrimaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["EndSecondaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHideWorldModelInFirstPerson"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsAutomaticPrimary"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAutomaticSecondary"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Deploy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsDeployed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrimaryAmmoType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSecondaryAmmoType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSecondaryAmmoTypeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPrimaryClipEmpty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxSecondaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewModelOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasSecondaryAmmo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSecondaryClipEmpty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPrimaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasAmmo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSecondaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Reload"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSecondaryClipSizeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxPrimaryClipSizeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxSecondaryClipSizeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Holster"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxSecondaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EndAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["EndPrimaryAttack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxPrimaryClipSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddPrimaryClip"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsInFirstPersonMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlayViewActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetViewModel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHideWorldModelInFirstPerson"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetViewModelOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetViewFOV"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetViewFOV"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SoundDspDistortionComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoundDspEAXReverbComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["IteratorFilterNameOrClass"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["SoundDspEchoComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SmokeTrailComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointConstraintFixedComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PropComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoundDspFlangerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["CameraComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["UpdateViewMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNearZProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFarPlaneBoundaries"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateProjectionMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFOV"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetProjectionMatrixProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewMatrixProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFarZProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFOVProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAspectRatioProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAspectRatio"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNearZ"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPlaneBoundaries"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFarZ"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFrustumNeighbors"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFOV"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFOVRad"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNearZ"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAspectRatio"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFarZ"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrustumPlanes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrustumPoints"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFarPlaneCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookAt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNearPlaneCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DepthToDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPlaneCenter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNearPlaneBoundaries"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetProjectionMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetViewMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNearPlaneBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFarPlaneBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNearPlanePoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFarPlanePoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPlanePoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFrustumPlaneCornerPoints"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["CreateFrustumKDop"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["CreateFrustumMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ScreenSpaceToWorldSpace"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WorldSpaceToScreenSpace"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WorldSpaceToScreenSpaceDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CalcScreenSpaceDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CalcRayDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetProjectionMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateMatrices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["GlobalComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetGlobalName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGlobalName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Animated2Component"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearAnimationManagers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveAnimationManager"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPlaybackRateProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAnimationManager"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAnimationManagers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAnimationManager"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["IteratorFilter"] = {
				type = "class",
				description = [[]],
				childs = {

				}
			},
			["VehicleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetSpeedKmh"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSteeringFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSteeringWheel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasDriver"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDriver"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDriver"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearDriver"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPhysicsVehicle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetupSteeringWheel"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetupPhysics"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["ColorComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetColorProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ScoreComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetScoreProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScore"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScore"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddScore"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SubtractScore"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["LightSpotComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetOuterCutoffAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOuterCutoffAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetInnerCutoffAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInnerCutoffAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["MicrophoneComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["QuakeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoundScapeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["WindComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["RasterizationRendererComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["RecordLightingPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPrepassDepthTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrepassNormalTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSSAOEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRenderTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShaderOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["BeginRenderPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearShaderOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EndRenderPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPostPrepassDepthTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLightingPassCommandBufferRecorder"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPrepassShader"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPrepassMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPrepassMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPostProcessingDepthDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPostProcessingHDRColorDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBloomTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRenderTargetTextureDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReloadPresentationRenderTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ScheduleMeshForRendering"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordPrepass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ExecutePrepass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ExecuteLightingPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPrepassCommandBufferRecorder"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetShadowCommandBufferRecorder"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["UpdatePrepassRenderBuffers"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["UpdateLightingPassRenderBuffers"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLightSourceDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSSAOEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ButtonComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointConstraintConeTwistComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DecalComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["CreateFromProjection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DebugDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["PointConstraintDoFComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointConstraintHingeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PathNodeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["VelocityComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetVelocityProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularVelocityProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocalAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddLocalAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocalVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddLocalVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["PointTargetComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ViewBodyComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PropPhysicsComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["FireComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["AttachableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["ClearAttachment"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocalPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AttachToBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AttachToAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLocalPose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBone"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetAttachmentFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttachmentFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AttachToEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AttachmentInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetRotation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetOffset"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["flags"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["TriggerGravityComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["TriggerHurtComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["TriggerPushComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["TriggerRemoveComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LightMapComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetLightmapTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ConvertLightmapToBSPLuxelData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateLightmapUvBuffers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLightmapAtlas"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReloadLightmapData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetExposure"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetExposure"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetExposureProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["BakeSettings"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColorTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["ResetColorTransform"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["exposure"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["height"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rebuildUvAtlas"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["samples"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["globalLightIntensityFactor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["denoise"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["createAsRenderJob"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["width"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["FogControllerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["TouchComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetTriggerFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTriggerFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTouchingEntityCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetTouchingEntities"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["SkyboxComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetSkyAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSkyAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["IteratorFilterModel"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ModelComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetModel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSkin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSkin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxDrawDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSkinProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRandomSkin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBodyGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRenderMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBodyGroups"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetModelName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupBlendController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaterialOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["LookupAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHitboxCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHitboxBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttachmentTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLOD"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaterialOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearMaterialOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxDrawDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["CSMComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["BaseEntity"] = {
				type = "class",
				inherits = "ents.Entity",
				description = [[]],
				childs = {
					["Initialize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BaseEntityComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Initialize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnTick"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnRemove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnEntitySpawn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnAttachedToEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNetworked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnDetachedFromEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsNetworked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShouldTransmitSnapshotData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ShouldTransmitSnapshotData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddEntityComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetVersion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FlagCallbackForRemoval"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["BindComponentInitEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["BindEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEntityComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReceiveNetEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SendSnapshotData"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnMemberValueChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SendData"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReceiveData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReceiveSnapshotData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["PlayerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetObserverTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetActionInputAxisMagnitude"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsFlashlightEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsKeyDown"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetViewPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetObserverTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFlashlightEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToggleFlashlight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActionInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PrintMessage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Respawn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Kick"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyViewRotationOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRunSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetActionInputs"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetActionInputAxisMagnitude"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetActionInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SendResource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetUtilityDrone"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(void)"
					},
					["GetTimeConnected"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWalkSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRunSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCrouchedWalkSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStandEyeLevel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSprintSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStandEyeLevel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCrouchHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCrouchHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCrouchedWalkSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocalPlayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStandHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStandHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSprintSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetObserverMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWalkSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCrouchEyeLevel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCrouchEyeLevel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetObserverMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetObserverModeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInFirstPersonMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetViewOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["RaytracingRendererComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["WaterComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["CreateSplash"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetStiffness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTorqueDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStiffness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPropagation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPropagation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWaterVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDensity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetWaterVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTorqueDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcLineSurfaceIntersection"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetWaterPlane"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ProjectToSurface"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWaterSceneDepthTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetReflectionScene"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWaterSceneTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["FuncPhysicsComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["KinematicComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["BrushComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["FilterClassComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["ShouldPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["IteratorFilterName"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["OwnableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetOwner"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOwner"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DebugTextComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["GenericComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DebugPointComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DebugLineComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DebugBoxComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DebugSphereComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["DebugPlaneComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PointAtTargetComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetPointAtTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPointAtTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IteratorFilterEntity"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["InfoLandmarkComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["HealthComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetMaxHealthProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHealthProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHealth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxHealth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHealth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxHealth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["NameComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNameProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IteratorFilterSphere"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["NetworkedComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["IteratorFilterComponent"] = {
				type = "class",
				inherits = "ents.IteratorFilter",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["ShooterComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["FireBullets"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["PhysicsComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["IsPhysicsProp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPhysicsObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCollisionCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionContactReportEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMoveType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionExtents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InitializePhysics"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyTorqueImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DestroyPhysicsObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DropToFloor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTrigger"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSimulationEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPhysJointConstraints"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetKinematic"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsKinematic"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAABBDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionCallbacksEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionCallbacksEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCollisionContactReportEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["EnableCollisions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DisableCollisions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSimulationEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ResetCollisions"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRotatedCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyTorque"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPhysicsType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionRadius"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsRagdoll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Entity"] = {
				type = "class",
				description = [[]],
				childs = {
					["RemoveComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Spawn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsNPC"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsCharacter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPlayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CreateSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["EmitSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSnapshotDirty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsMapEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsWorld"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetKeyValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Disable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveSafely"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInert"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetClass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsScripted"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsStatic"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsSpawned"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Load"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsVehicle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveEntityOnRemoval"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsDisabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["BroadcastNetEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPhysicsComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetModelComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearComponents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SendNetEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWeaponComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["C"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetComponents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TakeDamage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetModel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGenericComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPlayerComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSynchronized"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTimeScaleComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAIComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUuid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveFromScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUuid"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDynamic"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttachmentPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetActivity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Enable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsShared"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShared"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSynchronized"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddNetworkedComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetOwner"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsTurnedOn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PlayActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PlayAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOwner"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHealth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHealth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxHealth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxHealth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTransformComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTurnedOn"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCharacterComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnimatedComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNameComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetComponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSpawnFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetModelName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSkin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPhysicsObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InitializePhysics"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DestroyPhysicsObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DropToFloor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSkin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["TurnOn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TurnOff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTurnedOff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Input"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlayLayeredActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["PlayLayeredAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["StopLayeredAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAirDensity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVehicleComponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsInScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsClientsideOnly"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetClientIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSceneFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddToScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveFromAllScenes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ExplosionComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LightComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetFalloffExponent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLightIntensityCandela"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFalloffExponent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLightIntensityType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLightIntensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLightIntensityType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLightIntensity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateBuffers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAddToGameScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShadowType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShadowType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMorphTargetsInShadowsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AreMorphTargetsInShadowsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["SubmergibleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["IsSubmerged"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubmergedFraction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInWater"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFullySubmerged"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubmergedFractionProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWaterEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["MapComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetMapIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["UsableComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LogicComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["IKComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetIKControllerEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsIKControllerEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetIKEffectorPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetIKEffectorPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["vrp"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["ShadowManagerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["EyeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetEyePose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetViewTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEyeballState"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearViewTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEyeJitter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBlinkingEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEyeShift"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEyeShift"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetEyeJitter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEyeSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEyeSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetIrisDilation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetIrisDilation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CalcEyeballPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBlinkDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBlinkDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBlinkingEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EyeballState"] = {
						type = "class",
						description = [[]],
						childs = {
							["up"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["origin"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["irisProjectionV"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["irisProjectionU"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["forward"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["right"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["EnvTimescaleComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SkyCameraComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ShadowMapComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["PBRConverterComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GenerateAmbientOcclusionMaps"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BSPComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["BSPLeafComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ViewModelComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetPlayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetWeapon"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["RaytracingComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["WaterSurfaceComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["ListenerComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["SoftBodyComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["RendererComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["InitializeRenderTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetHDRPresentationTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSceneTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPresentationTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["WeatherComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {

				}
			},
			["LightMapReceiverComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["UpdateLightmapUvData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["VertexAnimatedComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["UpdateVertexAnimationBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVertexAnimationBufferMeshOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVertexAnimationBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalVertexPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["FlexComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["GetFlexAnimationCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetFlexController"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetScaledFlexController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlexControllerScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexControllerScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcFlexValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexAnimationCycle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFlexWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexWeights"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFlexWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearFlexWeightOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasFlexWeightOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlexWeightOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["PlayFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["StopFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlexAnimationCycle"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetFlexAnimationPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFlexController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["ReflectionProbeComponent"] = {
				type = "class",
				inherits = "ents.EntityComponent",
				description = [[]],
				childs = {
					["SetIBLStrength"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetIBLMaterialFilePath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CaptureIBLReflectionsFromScene"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["RequiresRebuild"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetIBLStrength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["_G"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Quaternion"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_script_path"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["e"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["class_names"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["ec"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["exec"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["class_info"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["include"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["phys"] = {
		type = "lib",
		description = [[]],
		childs = {
			["ScaledTransform"] = {
				type = "class",
				inherits = "phys.Transform",
				description = [[]],
				childs = {
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["CompoundShape"] = {
				type = "class",
				inherits = "phys.Shape phys.Base",
				description = [[]],
				childs = {

				}
			},
			["RayCastResult"] = {
				type = "class",
				description = [[]],
				childs = {
					["normal"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["distance"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["hitType"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["position"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physObj"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["fraction"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["startPosition"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["colObj"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["subMesh"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["material"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["materialName"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mesh"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["entity"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["IKController"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetKeyValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChainLength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEffectorName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetKeyValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetEffectorName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetKeyValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMethod"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMethod"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetKeyValues"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetChainLength"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ContactInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["shape0"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["GetContactPoints"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["shape1"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["GetContactPoint"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["collisionObj0"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["GetContactPointCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["collisionObj1"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Transform"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetOrigin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetIdentity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsIdentity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["TranslateGlobal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["TranslateLocal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RotateGlobal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RotateLocal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["TransformGlobal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["TransformLocal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Interpolate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InterpolateToIdentity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["pitch"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["yaw"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["roll"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["TriangleShape"] = {
				type = "class",
				inherits = "phys.Shape phys.Base",
				description = [[]],
				childs = {

				}
			},
			["Shape"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsConvex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalculateLocalInertia"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsConvexHull"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsHeightfield"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTriangleShape"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Heightfield"] = {
				type = "class",
				inherits = "phys.Shape phys.Base",
				description = [[]],
				childs = {
					["GetUpAxis"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMaxHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["RayCastData"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetTargetOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDirection"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShape"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTargetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSourceRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSourceTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["RigidBody"] = {
				type = "class",
				inherits = "phys.CollisionObj phys.Base",
				description = [[]],
				childs = {
					["SetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTotalTorque"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInertia"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetInvInertiaTensorWorld"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearForces"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTotalForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneID"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBoneID"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetKinematic"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetAngularSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSleepingThresholds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCenterOfMassOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCenterOfMassOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsKinematic"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ApplyTorque"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyTorqueImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["WheelCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["maxHandBrakeTorque"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxSteeringAngle"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tireType"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["radius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["suspension"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["width"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["shapeIndex"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["momentOfInertia"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["chassisOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["CollisionObj"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["IsAsleep"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAlwaysAwake"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsRigidBody"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Spawn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsGhostObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShape"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["EnableSimulation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOrigin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAlwaysAwake"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsSoftBody"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSimulationEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsSimulationEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoundingSphere"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DisableSimulation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActivationState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WakeUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetActivationState"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWorldTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWorldTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PutToSleep"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DoFConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {
					["SetLinearLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLinearMotorEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCurrentAngularPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentAngularLimitError"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearMaxMotorForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetlinearUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularLimitSoftness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularTargetVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularMaxMotorForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularMaxMotorForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularMaxLimitForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularMaxLimitForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearRestitutionFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAngularMotorEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularLimitSoftness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularLimitErrorTolerance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularLimitErrorTolerance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularLimitForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearTargetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularLimitForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularRestitutionFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularRestitutionFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularMotorEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearRestitutionFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentAngularLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearLimitForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentAngularAccumulatedImpulse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularTargetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearTargetVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearMaxMotorForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearLimitSoftness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearLimitSoftness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLinearLimitErrorTolerance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearLimitErrorTolerance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearLimitForceMixingFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearMotorEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCurrentLinearDifference"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentLinearLimitError"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentLinearLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentLinearAccumulatedImpulse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IKTree"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetEffector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSuccessor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEffectorPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRoot"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UnFreeze"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNodeCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEffectorCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetJointCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Compute"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Print"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InsertRightSibling"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InsertRoot"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InsertLeftChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetJoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Node"] = {
						type = "class",
						description = [[]],
						childs = {
							["UpdateTheta"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLeftChildNode"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetRightChildNode"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["InitNode"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsFrozen"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddToTheta"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLocalTransform"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetAttach"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRelativePosition"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTheta"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetS"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetW"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetMinTheta"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetMaxTheta"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetRestAngle"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTheta"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ComputeS"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ComputeW"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsEffector"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsJoint"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetEffectorIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Freeze"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["UnFreeze"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetJointIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAttach"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["PrintNode"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetRotationAxis"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetRotationAxis"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},

				}
			},
			["Material"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["SetDynamicFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStaticFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSurfaceMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDynamicFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRestitution"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRestitution"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSurfaceMaterial"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStaticFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ConvexShape"] = {
				type = "class",
				inherits = "phys.Shape phys.Base",
				description = [[]],
				childs = {
					["GetCollisionMesh"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["VehicleCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AddWheel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWheels"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAntiRollBar"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAntiRollBars"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["gearSwitchTime"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["chassis"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["clutchStrength"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["gravityFactor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["wheelDrive"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxEngineRotationSpeed"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["actor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxEngineTorque"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["SuspensionInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["springDamperRate"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["camberAngleAtRest"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["camberAngleAtMaxDroop"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["maxDroop"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["camberAngleAtMaxCompression"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["maxCompression"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["springStrength"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Base"] = {
				type = "class",
				description = [[]],
				childs = {

				}
			},
			["Controller"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSlopeLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDimensions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddMoveVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Spawn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Move"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMoveVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundShape"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundBody"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundMaterial"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTouchingGround"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundTouchPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFootPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundTouchNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStepHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDimensions"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFootPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUpDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUpDirection"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStepHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShape"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSlopeLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vehicle"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["SetGear"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGearUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGearSwitchTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUseDigitalInputs"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetForwardSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ChangeToGear"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUseAutoGears"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ShouldUseAutoGears"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentGear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSteerFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEngineRotationSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRestState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWheelCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ResetControls"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWheelRotationAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetWheelRotationSpeed"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsInAir"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSidewaysSpeed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGearDown"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBrakeFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHandBrakeFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAccelerationFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BoxShape"] = {
				type = "class",
				inherits = "phys.ConvexShape phys.Shape phys.Base",
				description = [[]],
				childs = {
					["GetHalfExtents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ConvexHullShape"] = {
				type = "class",
				inherits = "phys.ConvexShape phys.Shape phys.Base",
				description = [[]],
				childs = {
					["AddTriangle"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["ReservePoints"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddPoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Build"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReserveTriangles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["CapsuleShape"] = {
				type = "class",
				inherits = "phys.ConvexShape phys.Shape phys.Base",
				description = [[]],
				childs = {
					["GetRadius"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHalfHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["GhostObj"] = {
				type = "class",
				inherits = "phys.Base",
				description = [[]],
				childs = {
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IKJacobian"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetColumnCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRowCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetJtargetActive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ComputeJacobian"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CalcDeltaThetas"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetJendActive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ZeroDeltaThetas"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcDeltaThetasTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcDeltaThetasPseudoinverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcDeltaThetasDLS"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcDeltaThetasDLSwithSVD"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcDeltaThetasSDLS"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateThetas"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdatedSClampValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["UpdateThetaDot"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Constraint"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetBreakTorque"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceActor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetActor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourcePosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBreakForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBreakForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBreakTorque"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EnableCollisions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DisableCollisions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTargetTransform"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SoftBody"] = {
				type = "class",
				inherits = "phys.CollisionObj phys.Base",
				description = [[]],
				childs = {
					["SetVolumeConversationCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPressureCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVolumeMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVolume"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRigidContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnchorsHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetKineticContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDynamicFrictionCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDampingCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftVsRigidImpulseSplitS"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLiftCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDynamicFrictionCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPoseMatchingCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddAeroForceToFace"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetSoftVsKineticHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRestitution"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftVsRigidImpulseSplitK"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVelocitiesCorrectionFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVolumeDensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftVsRigidHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LocalVertexIndexToNodeIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRestitution"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSoftVsRigidImpulseSplitR"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftVsSoftHardness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnchorsHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRigidContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRollingFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDampingCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MeshVertexIndexToLocalVertexIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LocalVertexIndexToMeshVertexIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRestLengthScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddAeroForceToNode"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftVsRigidHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetKineticContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHitFraction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnisotropicFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaterialVolumeStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSoftVsSoftHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftVsRigidImpulseSplitS"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVolumeConversationCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftVsRigidImpulseSplitK"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVelocitiesCorrectionFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLiftCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaterialAngularStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaterialLinearStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaterialVolumeStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMaterialAngularStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaterialLinearStiffnessCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSoftVsKineticHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AppendAnchor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["NodeIndexToLocalVertexIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MeshVertexIndexToNodeIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["NodeIndexToMeshVertexIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSoftContactsHardness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNodeCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftVsRigidImpulseSplitR"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPressureCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPoseMatchingCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHitFraction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnisotropicFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRollingFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRestLengthScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWindVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWindVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["SoftBodyInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetMaterialStiffnessCoefficients"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetMaterialStiffnessCoefficients"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["clusterCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["poseMatchingCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dragCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxClusterIterations"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["anchorsHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["rigidContactsHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softContactsHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["liftCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["kineticContactsHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dynamicFrictionCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsRigidImpulseSplitK"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dampingCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["volumeConversationCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsRigidImpulseSplitR"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsRigidImpulseSplitS"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsKineticHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsRigidHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["softVsSoftHardness"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["pressureCoefficient"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["velocitiesCorrectionFactor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["bendingConstraintsDistance"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["FixedConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {

				}
			},
			["Object"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundPhysObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionObjects"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTotalTorque"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearForces"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTotalForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundSurfaceMaterial"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsOnGround"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGroundFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSleepingThresholds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddLinearVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroundVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGroundPhysCollisionObject"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetGroundEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionFilterMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsGroundWalkable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionFilterGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOwner"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularSleepingThreshold"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PutToSleep"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WakeUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ApplyTorque"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyTorqueImpulse"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["HingeConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {

				}
			},
			["ContactPoint"] = {
				type = "class",
				description = [[]],
				childs = {
					["normal"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["position"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["distance"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["impulse"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["material0"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["material1"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["SliderConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {
					["GetRestitution"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRestitution"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DisableLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLimits"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLimits"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDamping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ConeTwistConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {
					["SetLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DoFSpringConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {
					["CalculateTransforms"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetERP"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCalculatedTransformA"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetServoTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetCalculatedTransformB"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrameOffsetA"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAxis"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRelativePivotPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFrameOffsetB"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFrames"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLinearLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAxis"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLinearUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStopCFM"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLinearUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTargetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetAngularLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDamping"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetAngularLowerLimitReversed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularLowerLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAngularLowerLimitReversed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAngularUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngularUpperLimitReversed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngularUpperLimitReversed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLimitReversed"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["IsLimited"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetStopCFM"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetRotationOrder"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRotationOrder"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBounce"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["EnableMotor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetServo"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetMaxMotorForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["EnableSpring"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetEquilibriumPoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetERP"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetStopERP"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetStopERP"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetCFM"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetCFM"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetStiffness"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetAngularUpperLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BallSocketConstraint"] = {
				type = "class",
				inherits = "phys.Constraint",
				description = [[]],
				childs = {

				}
			},
			["ChassisCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AddShapeIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShapeIndices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["momentOfInertia"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["centerOfMass"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["coroutine"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["doc"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Collection"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetDescription"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFunctions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDerivedFrom"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMembers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRelated"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEnumSets"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Group"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Member"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGameStateFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDefault"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDescription"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BaseCollectionObject"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Enum"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGameStateFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDescription"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["EnumSet"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUnderlyingType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEnums"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Module"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWikiURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DerivedFrom"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Overload"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetParameters"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Function"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetDescription"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetURL"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFullName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGameStateFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetExampleCode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOverloads"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRelated"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["resource"] = {
		type = "lib",
		description = [[]],
		childs = {
			["add_file"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["add_lua_file"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_list"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["prosper"] = {
		type = "lib",
		description = [[]],
		childs = {
			["create_buffer"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["util"] = {
				type = "lib",
				description = [[]],
				childs = {
					["get_square_vertex_count"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_vertex_format"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_line_vertex_buffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_uv_format"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_line_vertex_count"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["allocate_temporary_buffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_line_vertex_format"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_line_vertices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_vertex_uv_buffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_vertex_buffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_uv_buffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_vertices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["get_square_uv_coordinates"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["ImageSubresourceRange"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["baseArrayLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["levelCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["baseMipLevel"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["layerCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Mesh"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetVertexBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertexWeightBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAlphaBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertexWeightBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIndexBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAlphaBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearBuffers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetIndexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TextureCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["sampler"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["imageView"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["RenderPassInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AddClearValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRenderPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetClearValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLayer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TimerQuery"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ImageSubresourceLayers"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["aspectMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["layerCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mipLevel"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["baseArrayLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["ImageCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["samples"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["height"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["format"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tiling"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["postCreateLayout"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["layers"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["queueFamilyMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["type"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["width"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["usageFlags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["memoryFeatures"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["RenderPassCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AddAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["AddSubPass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSubPassColorAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetSubPassDepthStencilAttachmentEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSubPassDependency"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["RenderTarget"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetRenderPass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFramebuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Framebuffer"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SwapBuffer"] = {
				type = "class",
				description = [[]],
				childs = {

				}
			},
			["ClearImageInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["subresourceRange"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["PipelineBarrierInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["dstStageMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["srcStageMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BufferCopyInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["size"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["srcOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Image"] = {
				type = "class",
				description = [[]],
				childs = {
					["Convert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSampleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsNormalMap"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSrgb"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAspectSubresourceLayout"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSrgb"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAlignment"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetExtent2D"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMipmapSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLayerCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMipmapCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUsage"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSharingMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTiling"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetNormalMap"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMemory"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCreateInfo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToImageBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMemoryBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BlitInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["srcSubresourceLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstSubresourceLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["WindowCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["floating"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["stereo"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["srgbCapable"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["height"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["doublebuffer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["refreshRate"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["samples"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["greenBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["redBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["blueBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["alphaBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["depthBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["focused"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["stencilBits"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["title"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["width"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["resizable"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["visible"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["decorated"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["autoIconify"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BufferImageCopyInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["height"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstImageLayout"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["aspectMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["layerCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mipLevel"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["bufferOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["baseArrayLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["width"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BufferCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["size"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["memoryFeatures"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["queueFamilyMask"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["usageFlags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["RenderTargetCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["useLayerFramebuffers"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["SamplerCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["minFilter"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["addressModeW"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mipLodBias"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxAnisotropy"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["magFilter"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["addressModeV"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mipmapMode"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["addressModeU"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["compareEnable"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["compareOp"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["minLod"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxLod"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["borderColor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["ImageView"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetLayerCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAspectMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBaseLayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetImageFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBaseMipmapLevel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSwizzleArray"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMipmapCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ImageViewCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["swizzleBlue"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["swizzleAlpha"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["format"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mipmapLevels"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["baseLayer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["levelCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["swizzleRed"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["swizzleGreen"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["baseMipmap"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["ImageCopyInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["srcSubresource"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstSubresource"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["srcOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["height"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstImageLayout"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["width"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["dstOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["srcImageLayout"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["SubresourceLayout"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["rowPitch"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["arrayPitch"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["size"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["depthPitch"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["offset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Event"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Window"] = {
				type = "class",
				description = [[]],
				childs = {
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Close"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetWindowTitle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDecorated"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ShouldClose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetClipboardString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShouldClose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetClipboardString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetCursorPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStickyKeysEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStickyKeysEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetStickyMouseButtonsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStickyMouseButtonsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCursorPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SwapBuffers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsVisible"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFramebufferSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFrameSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsResizable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsFocused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Restore"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Show"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Hide"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearCursorPosOverride"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCursorPosOverride"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetKeyState"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsFloating"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsIconified"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClearCursor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMouseButtonState"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetCursorInputMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCursorInputMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetCloseCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCursor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Iconify"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MakeContextCurrent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ClearValue"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthStencil"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["CommandBufferRecorder"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPending"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ExecuteCommands"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["TimestampQuery"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DescriptorSet"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetBindingCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBindingTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBindingArrayTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetBindingUniformBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBindingUniformBufferDynamic"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBindingStorageBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Buffer"] = {
				type = "class",
				description = [[]],
				childs = {
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetUsageFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMemory"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBaseIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMemory"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCreateInfo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStartOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPermanentlyMapped"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["MapMemory"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["UnmapMemory"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["CommandBuffer"] = {
				type = "class",
				description = [[]],
				childs = {
					["RecordDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBlitTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordSetViewport"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordSetScissor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordGenerateMipmaps"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordPipelineBarrier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordSetDepthBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordBeginRenderPass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordImageBarrier"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordBufferBarrier"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordEndRenderPass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["StopRecording"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordDrawIndexedIndirect"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordCopyImageToBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordDrawIndirect"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordFillBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RecordSetBlendConstants"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordSetDepthBias"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RecordSetLineWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordDrawIndexed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordSetStencilReference"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordSetStencilWriteMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordUpdateBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RecordDrawGradient"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["StartRecording"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Flush"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordSetStencilCompareMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordResolveImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordBlitImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RecordClearAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordCopyBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RecordCopyBufferToImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordClearImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordCopyImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindVertexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordBindVertexBuffers"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RecordBindIndexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Fence"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["RenderPass"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Sampler"] = {
				type = "class",
				description = [[]],
				childs = {
					["Update"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinLod"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBorderColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMagFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxAnisotropy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCompareEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCompareOperation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxLod"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMinFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAddressModeU"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAddressModeW"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxAnisotropy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCompareEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCompareOperation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMipLodBias"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMinLod"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAddressModeV"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMagFilter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMipmapMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxLod"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBorderColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAddressModeU"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMipmapMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnisotropyEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMinFilter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAddressModeV"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAddressModeW"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Texture"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetImage"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetImageView"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSampler"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetImageView"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["bit"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["math"] = {
		type = "lib",
		description = [[]],
		childs = {
			["approach"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["approach_angle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_angle_difference"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_angle_in_range"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clamp_angle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clamp"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_ballistic_angle_of_reach"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_next_power_of_2"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_previous_power_of_2"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["smooth_step"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["smoother_step"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_ballistic_range"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_ballistic_position"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_frustum_plane_center"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_average_rotation"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["map_value_to_fraction"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["map_value_to_range"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["Quaternion"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["NoiseModule"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Checkerboard"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Cylinders"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Const"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["RotatePoint"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Turbulance"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["ScaleBias"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Blend"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Spheres"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Billow"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {
							["GetFrequency"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFrequency"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetPersistence"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSeed"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPersistence"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSeed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Min"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Perlin"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {
							["GetFrequency"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFrequency"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetPersistence"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSeed"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPersistence"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSeed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Exponent"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Power"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Select"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["ScalePoint"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Curve"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["RidgedMulti"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {
							["GetFrequency"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLacunarity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFrequency"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSeed"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetNoiseQuality"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetOctaveCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSeed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Abs"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Multiply"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Displace"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Terrace"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Cache"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["TranslatePoint"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Invert"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Voronoi"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {
							["GetDisplacement"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetDisplacement"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSeed"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSeed"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFrequency"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFrequency"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["Clamp"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},
					["Max"] = {
						type = "class",
						inherits = "math.NoiseModule",
						description = [[]],
						childs = {

						}
					},

				}
			},
			["Mat2x4"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Vector4i"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["w"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Vectori"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Vector2"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RotateAround"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Cross"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Project"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Length"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["LengthSqr"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Distance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["DistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["QuaternionInternal"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetConjugate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Slerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ToMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetOrientation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Inverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToAxisAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RotateX"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RotateY"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RotateZ"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Distance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClampRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApproachDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Length"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["w"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Mat2x3"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["NoiseMap"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Transform"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["UpdateMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTransformationMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Vector2i"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["PIDController"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetIntegralTerm"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDerivativeTerm"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetTerms"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetProportionalTerm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetIntegralTerm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetDerivativeTerm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetTerms"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetProportionalTerm"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Calculate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClearRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetMax"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["RotateAround"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Cross"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Equals"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Project"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["PlanarDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Slerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SnapToGrid"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ProjectToPlane"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPerpendicular"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["OuterProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToScreenUv"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPitch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetYaw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlanarDistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Length"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["LengthSqr"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Distance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["DistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Vector4"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Project"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Length"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LengthSqr"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Distance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["a"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["w"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Mat3x4"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Mat4"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Decompose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MulRow"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SwapColumns"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ApplyProjectionDepthBiasOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetInverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Inverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SwapRows"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ToQuaternion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MulCol"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Mat4x2"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Mat4x3"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Plane"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCenterPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MoveToPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Transform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Mat3x2"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Mat3"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcEigenValues"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Inverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Mat2"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Transpose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTranspose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Inverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["EulerAngles"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Equals"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOrientation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToQuaternion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetForward"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["p"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["light"] = {
		type = "lib",
		description = [[]],
		childs = {
			["get_color_temperature"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["get_average_color_temperature"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["color_temperature_to_color"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["wavelength_to_color"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["lux_to_irradiance"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_luminous_efficacy"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["watts_to_lumens"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["irradiance_to_lux"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["lumens_to_watts"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["convert_light_intensity"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["game"] = {
		type = "lib",
		description = [[]],
		childs = {
			["change_map"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["set_gravity"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["get_gravity"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["load_sound_scripts"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["precache_model"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["load_model"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["load_material"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_model"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["load_texture"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_material"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_material"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["precache_material"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["get_error_material"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clear_unused_materials"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["precache_particle_system"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["limits"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["SurfaceMaterial"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHardImpactSound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSubsurfaceScatterColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWaveStiffness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAudioHighFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPBRMetalness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAudioLowFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetWaveStiffness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetWavePropagation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFootstepSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBulletImpactSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetWavePropagation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFootstepSound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPBRRoughness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLinearDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAudioScattering"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSubsurfaceFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSoftImpactSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSubsurfaceScatterColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubsurfaceRadiusMM"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAudioMidFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSubsurfaceRadiusMM"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetImpactParticleEffect"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSubsurfaceColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSubsurfaceColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSubsurfaceFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTorqueDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTorqueDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearIOR"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLinearDragCoefficient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDensity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNavigationFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStaticFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDynamicFriction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAudioHighFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStaticFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHardImpactSound"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDynamicFriction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAudioLowFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRestitution"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRestitution"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAudioScattering"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetImpactParticleEffect"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBulletImpactSound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftImpactSound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIOR"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAudioLowFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetIOR"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAudioLowFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAudioMidFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAudioHighFrequencyAbsorption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAudioMidFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAudioMidFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAudioHighFrequencyTransmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetNavigationFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["drone"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["BulletInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["distance"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerRadius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["damageType"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["damage"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["bulletCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["attacker"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["inflictor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerColor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerCount"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerLength"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerSpeed"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerBloom"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["ammoType"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["direction"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["effectOrigin"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tracerMaterial"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["force"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["spread"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Model"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetMass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetEyeOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlexAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFlexControllerCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SaveLegacy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEyeOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAttachmentData"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLODData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaterialNames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetBodyGroupMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFlexCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexControllers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveIKController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBaseMeshGroupIds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupFlexController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetObjectAttachmentCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlexes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupFlex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlexFormula"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexAnimationNames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CalcFlexWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["CalcReferenceAttachmentPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupBlendController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddHitbox"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["LookupAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnimationActivityWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHitboxBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCollisionMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIKControllers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIKController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddIKController"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["AddIncludeModel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AssignDistinctMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["ExportAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["LookupIKController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetIncludeModels"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearCollisionMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPhonemeMap"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPhonemeMap"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddBaseMeshGroupId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SelectWeightedAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetObjectAttachments"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaterialPaths"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasVertexWeights"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddObjectAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetEyeballCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLocalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["AddFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetTriangleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlexAnimationCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFlexAnimationName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClearFlexAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBlendController"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddMaterialPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSubMeshCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSkeleton"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttachmentCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttachments"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearMeshGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["RemoveAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddEyeball"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetObjectAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["TranslateLODMeshes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupObjectAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMeshGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["UpdateCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveObjectAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBlendControllerCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBlendControllers"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PrecacheTextureGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationNames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAnimationName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PrecacheTextureGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetReferencePose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMeshGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMeshes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBaseMeshGroupIds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMeshGroupId"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddMeshGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["UpdateRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddCollisionMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMaterials"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaterialIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMaterialCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMeshGroupCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionMeshCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBodyGroupId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBodyGroupCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHitboxGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHitboxBones"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHitboxGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetHitboxBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RemoveHitbox"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTextureGroupCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTextureGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTextureGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaterialPaths"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LoadMaterials"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveMaterialPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearMaterials"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["RemoveAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveMeshGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearJoints"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearBaseMeshGroupIds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddTextureGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLODCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SelectFirstAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLOD"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetJoints"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddJoint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetVertexAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddVertexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveVertexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBodyGroupMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMeshCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetCollisionBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Update"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVertexCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEyeballs"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsStatic"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasFlag"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsRootBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBodyGroups"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CalcReferenceBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHitboxCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBodyGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVertexAnimationBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Export"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Flex"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetOperations"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetVertexAnimation"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFrameIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexAnimation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Operation"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetName"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["index"] = {
										type = "value",
										description = [[]],
										valuetype = ""
									},
									["type"] = {
										type = "value",
										description = [[]],
										valuetype = ""
									},
									["value"] = {
										type = "value",
										description = [[]],
										valuetype = ""
									}
								}
							},

						}
					},
					["VertexAnimation"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetMeshAnimations"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["MeshAnimation"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetFrames"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetMesh"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Rotate"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["Frame"] = {
										type = "class",
										description = [[]],
										childs = {
											["Rotate"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["SetFlags"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["SetVertexPosition"] = {
												type = "function",
												description = [[]],
												args = "(: Function, : Function)",
												returns = "()"
											},
											["SetVertexNormal"] = {
												type = "function",
												description = [[]],
												args = "(: Function, : Function)",
												returns = "()"
											},
											["GetVertexNormal"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["GetVertexPosition"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["GetVertices"] = {
												type = "function",
												description = [[]],
												args = "()",
												returns = "()"
											},
											["GetVertexCount"] = {
												type = "function",
												description = [[]],
												args = "()",
												returns = "()"
											},
											["SetDeltaValue"] = {
												type = "function",
												description = [[]],
												args = "(: Function, : Function)",
												returns = "()"
											},
											["GetDeltaValue"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["SetVertexCount"] = {
												type = "function",
												description = [[]],
												args = "(: Function)",
												returns = "()"
											},
											["GetFlags"] = {
												type = "function",
												description = [[]],
												args = "()",
												returns = "()"
											}
										}
									},

								}
							},

						}
					},
					["MeshGroup"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__eq"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["AddMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetMeshCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetMeshes"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearMeshes"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetMeshes"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Animation"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetBoneList"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFrames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["CalcRenderBounds"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBoneWeight"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Reverse"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetActivityName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetBoneCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetFrame"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFrameCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBoneList"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["LookupBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["AddFrame"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Scale"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBoneWeight"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearFrames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetEventArgs"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetBoneId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["RemoveEvent"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["AddBoneId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetEventData"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function, : Function)",
								returns = "()"
							},
							["Translate"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetFadeInTime"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetFadeOutTime"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRenderBounds"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetActivity"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetFadeOutTime"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetEventType"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetBlendController"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBoneWeights"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Save"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SaveLegacy"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFadeInTime"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetEventCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetEvents"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddEvent"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetDuration"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["RemoveFlags"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["AddFlags"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetFlags"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFlags"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetActivityWeight"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetFPS"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFPS"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetActivityWeight"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetActivity"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Frame"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetBoneMatrix"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetLocalBoneTransform"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetBoneTransform"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetBoneRotation"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["Rotate"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function, : Function)",
										returns = "()"
									},
									["GetFlexControllerIds"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Localize"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetBonePosition"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["Globalize"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["Translate"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function, : Function)",
										returns = "()"
									},
									["GetMoveTranslation"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Scale"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetMoveTranslationX"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetMoveTranslationZ"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetFlexControllerWeights"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetMoveTranslation"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["SetFlexControllerWeights"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetMoveTranslationX"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetBoneTransform"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function, : Function)",
										returns = "()"
									},
									["GetBoneRotation"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetMoveTranslationZ"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetBoneScale"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["Copy"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetBonePose"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetBonePose"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["SetBoneCount"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetBoneCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetBoneScale"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["CalcRenderBounds"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									}
								}
							},

						}
					},
					["Eyeball"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetUpperLidFlexIndices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetUpperLidFlexAngles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetLowerLidFlexIndices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetLowerLidFlexAngles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetLowerLidFlexIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetUpperLidFlexIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["boneIndex"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["zOffset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["forward"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["up"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["irisMaterialIndex"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["name"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["irisUvRadius"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["irisScale"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["radius"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["maxDilationFactor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["origin"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["VertexWeight"] = {
						type = "class",
						description = [[]],
						childs = {
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__eq"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Copy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["boneIds"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["weights"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["Skeleton"] = {
						type = "class",
						description = [[]],
						childs = {
							["IsRootBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["Merge"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["LookupBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRootBones"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBoneCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBones"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearBones"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["MakeRootBone"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBoneHierarchy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["Bone"] = {
								type = "class",
								description = [[]],
								childs = {
									["__tostring"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetParent"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["IsDescendantOf"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["SetName"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetID"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["ClearParent"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetParent"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["IsAncestorOf"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["GetChildren"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetName"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									}
								}
							},

						}
					},
					["Mesh"] = {
						type = "class",
						description = [[]],
						childs = {
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddSubMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetCenter"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetSubMeshes"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTriangleCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["RemoveSubMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Update"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertexCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Translate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Scale"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetCenter"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearSubMeshes"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBounds"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__eq"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSubMeshCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSubMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSubMeshes"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTriangleVertexCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Centralize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetReferenceId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Sub"] = {
								type = "class",
								description = [[]],
								childs = {
									["__tostring"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetSkinTextureIndex"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GenerateNormals"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["ReserveVertexWeights"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["AddPoint"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetVertexWeights"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Rotate"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["FlipTriangleWindingOrder"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Copy"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["ReserveVertices"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexCount"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ReserveTriangles"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetTriangleVertexCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetIndexCount"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetTriangleCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetCenter"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["AddUVSet"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetUVs"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["GetUVSetNames"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["GetVertices"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetVertexCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["AddTriangle"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function, : Function)",
										returns = "()"
									},
									["GetNormals"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetVertexWeight"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetReferenceId"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetSkinTextureIndex"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexAlpha"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["Transform"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetBounds"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetVertex"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["SetVertexUV"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["AddVertex"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ClearExtendedVertexWeights"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetVertexNormal"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetVertex"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ClearVertexWeights"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetVertexUV"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetVertexAlpha"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetVertexWeight"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ApplyUVMapping"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
										returns = "()"
									},
									["SetVertexTangent"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetGeometryType"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetGeometryType"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["AddLine"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["ClearAlphas"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["ClearUVSets"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["HasUVSet"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetVertexPosition"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetTriangles"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["ClipAgainstPlane"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["Translate"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["Scale"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ClearVertices"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["ClearTriangles"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["__eq"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["ClearVertexData"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["SetPose"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexPosition"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetVertexNormal"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetPose"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["NormalizeUVCoordinates"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Optimize"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Update"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									}
								}
							},

						}
					},
					["Joint"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetKeyValues"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetChildBoneId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetCollisionsEnabled"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetType"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetArgs"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["SetKeyValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetArgs"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["RemoveKeyValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetParentBoneId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCollisionsEnabled"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetKeyValues"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetType"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetCollisionMeshId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetParentCollisionMeshId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["collide"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["FlexAnimation"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetFps"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFlexControllerIds"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["AddFlexControllerId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetFrames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetFrame"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetFrameCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["SetFlexControllerIds"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFlexControllerCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["SaveLegacy"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["LookupLocalFlexControllerIndex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["AddFrame"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["ClearFrames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["RemoveFrame"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Save"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetFlexControllerValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetFps"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["Frame"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetFlexControllerValues"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["SetFlexControllerValues"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetFlexControllerValue"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetFlexControllerValueCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "(: Function)",
										valuetype = "Function"
									},
									["SetFlexControllerValue"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									}
								}
							},

						}
					},
					["CollisionMesh"] = {
						type = "class",
						description = [[]],
						childs = {
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetOrigin"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAABB"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Copy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTriangles"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBoneParentId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Centralize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetShape"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBoneParentId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetConvex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddTriangle"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetSurfaceMaterialId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSurfaceMaterialIds"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IntersectAABB"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["IsConvex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetSoftBodyMesh"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetAABB"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetOrigin"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSurfaceMaterialId"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Update"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddVertex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTriangles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetSoftBodyTriangles"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClipAgainstPlane"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function, : Function)",
								returns = "()"
							},
							["ClearSoftBodyAnchors"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Translate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["IsSoftBody"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVolume"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearVertices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddSoftBodyAnchor"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetVolume"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearTriangles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetVertices"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetSoftBody"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSoftBodyMesh"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSoftBodyInfo"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSoftBodyTriangles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["RemoveSoftBodyAnchor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSoftBodyAnchors"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["Vertex"] = {
						type = "class",
						description = [[]],
						childs = {
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__eq"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Copy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBiTangent"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["tangent"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["position"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["normal"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["uv"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["ExportInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetAnimationList"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["exportAnimations"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["enableExtendedDDS"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["exportSkinnedMeshData"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["exportMorphTargets"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["exportImages"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["embedAnimations"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["normalizeTextureNames"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["fullExport"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["saveAsBinary"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["verbose"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["generateAo"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["aoSamples"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["aoResolution"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["scale"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mergeMeshesByMaterial"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["imageFormat"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["aoDevice"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["DamageInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetHitPosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHitGroup"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHitPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHitGroup"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetForce"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSource"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetForce"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDamage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddDamage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ScaleDamage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDamage"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttacker"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAttacker"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInflictor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetInflictor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDamageTypes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDamageType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddDamageType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveDamageType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDamageType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Material"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSpriteSheetAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsError"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShaderName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsTranslucent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDataBlock"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsLoaded"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["UpdateTextures"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearSpriteSheetAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetTextureInfo"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAlphaCutoff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetShader"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InitializeShaderDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SpriteSheetAnimation"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetSequenceCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSequence"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSequences"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Sequence"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetFrameCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetInterpolatedFrameData"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetDuration"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetFrame"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetFrames"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["IsLooping"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetFrameOffset"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Frame"] = {
										type = "class",
										description = [[]],
										childs = {
											["GetDuration"] = {
												type = "function",
												description = [[]],
												args = "()",
												returns = "()"
											},
											["GetUVBounds"] = {
												type = "function",
												description = [[]],
												args = "()",
												returns = "()"
											}
										}
									},

								}
							},

						}
					},

				}
			},
			["GibletCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["model"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["position"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["velocity"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["skin"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["scale"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mass"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["lifetime"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["rotation"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["angularVelocity"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physTranslationOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physRotationOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physHeight"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physShape"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["physRadius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["DepthStageRenderProcessor"] = {
				type = "class",
				inherits = "game.BaseRenderProcessor",
				description = [[]],
				childs = {
					["Render"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["RenderQueue"] = {
				type = "class",
				description = [[]],
				childs = {
					["IsComplete"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WaitForCompletion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["LightingStageRenderProcessor"] = {
				type = "class",
				inherits = "game.BaseRenderProcessor",
				description = [[]],
				childs = {
					["Render"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BaseRenderProcessor"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetDepthBias"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["RenderPassStats"] = {
				type = "class",
				description = [[]],
				childs = {
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["RenderStats"] = {
				type = "class",
				description = [[]],
				childs = {
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPassStats"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["WorldEnvironment"] = {
				type = "class",
				description = [[]],
				childs = {
					["IsUnlit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShaderQuality"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFogStart"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFogEnd"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFogStart"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShaderQualityProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogEnd"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFogEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFogColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAmbientColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsFogEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFogDensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFogDensity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFogType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFogType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogFarDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogEnabledProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogTypeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogMaxDensityProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogEndProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShaderQuality"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFogStartProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFogColorProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShadowResolution"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShadowResolutionProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAmbientColorProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetUnlitProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAmbientColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetUnlit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShadowResolution"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DrawSceneInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEntityRenderFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEntityPrepassFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["commandBuffer"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["renderFlags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["outputLayerId"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["renderStats"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["outputImage"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["clearColor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["renderTarget"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["scene"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["toneMapping"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["steamworks"] = {
		type = "lib",
		description = [[]],
		childs = {
			["workshop"] = {
				type = "lib",
				description = [[]],
				childs = {
					["get_item"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["create_item"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["add_item_dependency"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["add_item_app_dependency"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["remove_item_dependency"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["remove_item_app_dependency"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["subscribe_item"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["unsubscribe_item"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["get_subscribed_items"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["register_callback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Item"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPreviewImage"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetDownVoteCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTags"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCategoryFlags"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetDescription"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVisibility"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetCategoryFlags"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTimeAddedToUserList"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetChildCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTags"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetAddonContent"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["IsValid"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTimeCreated"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetDescription"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetURL"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetUpdateLanguage"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetProgress"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetType"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["PublishChanges"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetStateFlags"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Download"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetScore"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetInstallInfo"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetDownloadInfo"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSteamIdOwner"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTitle"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTimeUpdated"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsBanned"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AreTagsTruncated"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVisibility"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetFileSize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetPreviewFileSize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetTitle"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetUpVoteCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},

				}
			},

		}
	},
	["import"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["console"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Var"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetHelpText"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFloat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBool"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddChangeCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInt"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDefault"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["intersect"] = {
		type = "lib",
		description = [[]],
		childs = {
			["aabb_with_sphere"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["point_in_aabb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["line_with_aabb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["line_with_obb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["sphere_with_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["line_with_mesh"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["point_in_plane_mesh"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["line_with_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["aabb_with_triangle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["sphere_in_plane_mesh"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["aabb_in_plane_mesh"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["sphere_with_cone"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["line_with_triangle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["aabb_with_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["obb_with_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["line_with_sphere"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["aabb_with_aabb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["sphere_with_sphere"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["dmx"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Data"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetElements"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRootAttribute"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Attribute"] = {
				type = "class",
				description = [[]],
				childs = {
					["RemoveArrayValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValueAsString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddArrayValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Element"] = {
				type = "class",
				description = [[]],
				childs = {
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttribute"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttributes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttributeValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAttrV"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGUID"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttr"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["os"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["tf2"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["fudm"] = {
		type = "lib",
		description = [[]],
		childs = {
			["impl"] = {
				type = "lib",
				description = [[]],
				childs = {
					["name_to_type_id"] = {
						type = "lib",
						description = [[]],
						childs = {

						}
					},
					["registered_types"] = {
						type = "lib",
						description = [[]],
						childs = {

						}
					},
					["class_to_type_id"] = {
						type = "lib",
						description = [[]],
						childs = {

						}
					},

				}
			},

		}
	},
	["locale"] = {
		type = "lib",
		description = [[]],
		childs = {
			["change_language"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["set_text"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "()"
			},
			["get_language"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["load"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["ai"] = {
		type = "lib",
		description = [[]],
		childs = {
			["create_schedule"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["register_faction"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_faction_by_name"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_factions"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["register_task"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["Schedule"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetParameterEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetParameterEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Cancel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInterruptFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddInterruptFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetInterruptFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DebugPrint"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParameterType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetParameterBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetParameterEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddTask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRootTask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasParameter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetParameterInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["BaseBehaviorTask"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["Stop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Start"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Think"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["OnTaskComplete"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["OnSetScheduleParameter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["TaskPlayLayeredActivity"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnimationSlot"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskWait"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetMinWaitTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxWaitTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetWaitTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskMoveToTarget"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetMoveTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskDecorator"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetDecoratorType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Squad"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetMembers"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["MemoryFragment"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetLastTimeSensed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastCheckTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsInView"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastKnownPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastKnownVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastTimeSeen"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLastTimeHeared"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Memory"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetFragmentCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFragments"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["TaskEvent"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetEventId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEventArgument"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["TaskDebugDrawText"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetMessage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskDebugPrint"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetMessage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskRandom"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetChance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskPlaySound"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPitch"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoundName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BehaviorTask"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParameterEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsActive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTasks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetParameterVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSelectorType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParameterQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetTask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScheduleParameter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["LinkParameter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetDebugName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDebugName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParameterEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["CreateTask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CreateDecoratedTask"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParameterBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterType"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetParameterVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["HasParameter"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetParameterFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["TaskPlayLayeredAnimation"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnimationSlot"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskPlayActivity"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetFacePrimaryTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFaceTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TaskPlayAnimation"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFaceTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFacePrimaryTarget"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["MoveInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["destinationTolerance"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["moveOnPath"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["activity"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["faceTarget"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["turnSpeed"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["moveSpeed"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["TaskLookAtTarget"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetLookDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Faction"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetDisposition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetEnemyFaction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFearsomeFaction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAlliedFaction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetNeutralFaction"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDisposition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasClass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDefaultDisposition"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDefaultDisposition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddClass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetClasses"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["TaskMoveRandom"] = {
				type = "class",
				inherits = "ai.BehaviorTask",
				description = [[]],
				childs = {
					["SetMoveDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["debug"] = {
		type = "lib",
		description = [[]],
		childs = {
			["move_state_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["draw_point"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["beep"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["draw_box"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["npc_state_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["draw_line"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["draw_sphere"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["draw_cone"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "()"
			},
			["draw_truncated_cone"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["draw_cylinder"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["draw_pose"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["draw_text"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["draw_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "()"
			},
			["draw_spline"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["draw_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["stackdump"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["task_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["memory_type_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["disposition_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["behavior_task_decorator_type_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["behavior_task_type_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["behavior_task_result_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["behavior_selector_type_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["RendererObject"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetPose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsVisible"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVisible"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPose"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},

		}
	},
	["angle"] = {
		type = "lib",
		description = [[]],
		childs = {
			["random"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_from_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["geometry"] = {
		type = "lib",
		description = [[]],
		childs = {
			["closest_point_on_sphere_to_line"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_rotation_between_planes"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_barycentric_coordinates"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_side_of_point_to_line"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_side_of_point_to_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["triangulate"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["closest_points_between_lines"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["calc_point_on_triangle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_volume_of_polyhedron"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_outline_vertices"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["closest_point_on_aabb_to_point"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["closest_point_on_plane_to_point"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["closest_point_on_triangle_to_point"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["smallest_enclosing_sphere"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["closest_point_on_line_to_point"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_triangle_winding_order"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["generate_truncated_cone_mesh"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["calc_face_normal"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_volume_of_triangle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_center_of_mass"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["calc_triangle_area"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["noise"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["boundingvolume"] = {
		type = "lib",
		description = [[]],
		childs = {
			["get_rotated_aabb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			}
		}
	},
	["tool"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["ik"] = {
		type = "lib",
		description = [[]],
		childs = {
			["FABRIkSolver"] = {
				type = "class",
				inherits = "ik.IkSolver",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IkBallSocketConstraint"] = {
				type = "class",
				inherits = "ik.IkConstraint",
				description = [[]],
				childs = {
					["SetLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IkConstraint"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetJointIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["IkSolver"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetGlobalTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Solve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddHingeConstraint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetLocalTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLocalTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddCustomConstraint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddBallSocketConstraint"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["CCDIkSolver"] = {
				type = "class",
				inherits = "ik.IkSolver",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IkHingeConstraint"] = {
				type = "class",
				inherits = "ik.IkConstraint",
				description = [[]],
				childs = {
					["GetLimits"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLimits"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearLimits"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["Animation"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Frame"] = {
				type = "class",
				description = [[]],
				childs = {
					["CalcRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBoneScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMoveTranslationZ"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexControllerWeights"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMoveTranslationZ"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexControllerIds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetBoneRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Localize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Globalize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMoveTranslation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneMatrix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMoveTranslation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetFlexControllerWeights"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLocalBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetMoveTranslationX"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBoneTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetMoveTranslationX"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBonePosition"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBonePose"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["udm"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Property"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValueType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["HasValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAscii"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFromPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddArray"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetChildCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Add"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddAssetData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAssetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValuesFromBlob"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddBlobFromArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetBlobData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Struct"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetDescription"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["AssetData"] = {
				type = "class",
				inherits = "udm.LinkedPropertyWrapper",
				description = [[]],
				childs = {
					["GetAssetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAssetVersion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAssetType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAssetVersion"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["Array"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Element"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValueType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["HasValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAscii"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFromPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddArray"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetChildCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Add"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddAssetData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAssetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValuesFromBlob"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddBlobFromArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetBlobData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Reference"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["StructDescription"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetMemberCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetTypes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetNames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Srgba"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["a"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["HdrColor"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["a"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Data"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAscii"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAssetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ResolveReferences"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LoadProperty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SaveAscii"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRootElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["LinkedPropertyWrapper"] = {
				type = "class",
				inherits = "udm.PropertyWrapper",
				description = [[]],
				childs = {
					["GetAssetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClaimOwnership"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["PropertyWrapper"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValueType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["HasValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAscii"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFromPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddArray"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetChildCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["It"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Add"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddAssetData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToAssetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetArrayValuesFromBlob"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddBlobFromArrayValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetBlobData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["file"] = {
		type = "lib",
		description = [[]],
		childs = {
			["to_relative_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_size"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["delete"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["compare_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["remove_file_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["strip_illegal_filename_characters"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_directory"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["copy"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_file_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_file_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_file_name"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_canonicalized_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_absolute_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_flags"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_attributes"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_lua_files"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["read"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["write"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_directory"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["open_external_asset_file"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_external_game_asset_files"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["open"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["exists"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["File"] = {
				type = "class",
				description = [[]],
				childs = {
					["Close"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat2x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteChar"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Eof"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Read"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IgnoreComments"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Write"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteLongDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ReadChar"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBool"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadLongDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadFloat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Seek"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WriteVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadVector"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat2x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WriteMat3x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ReadInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Tell"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ReadQuaternion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WriteUInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat4x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ReadMat4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["WriteBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadUInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["sweep"] = {
		type = "lib",
		description = [[]],
		childs = {
			["aabb_with_aabb"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["aabb_with_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			}
		}
	},
	["package"] = {
		type = "lib",
		description = [[]],
		childs = {
			["preload"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["loaders"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},

		}
	},
	["input"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["asset"] = {
		type = "lib",
		description = [[]],
		childs = {
			["clear_unused_models"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clear_unused_materials"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_supported_import_file_extensions"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["matches"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clear_flagged_models"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["unlock_asset_watchers"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["flag_model_for_cache_removal"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["lock_asset_watchers"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["get_supported_export_file_extensions"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_normalized_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_supported_extensions"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_legacy_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_asset_root_directory"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_binary_udm_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_ascii_udm_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["determine_format_from_data"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["determine_format_from_filename"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["determine_type_from_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["matches_format"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["relative_path_to_absolute_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["absolute_path_to_relative_path"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_udm_format_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_file"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_loaded"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["exists"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["delete"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clear_unused_textures"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["TextureImportInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["srgb"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["normalMap"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["MapExportInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AddLightSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCamera"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["includeMapLightSources"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["engine"] = {
		type = "lib",
		description = [[]],
		childs = {
			["set_record_console_output"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["unbind_key"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["get_text_size"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_tick_count"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["shutdown"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["get_working_directory"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_current_frame_index"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["bind_key"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "()"
			},
			["set_fixed_frame_delta_time_interpretation"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["create_font"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_font"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clear_fixed_frame_delta_time_interpretation"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["get_window_resolution"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["set_tick_delta_time_tied_to_frame_rate"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["get_render_resolution"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_staging_render_target"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["FontInfo"] = {
				type = "class",
				description = [[]],
				childs = {

				}
			},

		}
	},
	["time"] = {
		type = "lib",
		description = [[]],
		childs = {
			["delta_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["time_since_epoch"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["convert_duration"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_timer"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["cur_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["real_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_simple_timer"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["server_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["frame_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["Timer"] = {
				type = "class",
				description = [[]],
				childs = {
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPaused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTimeLeft"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInterval"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetInterval"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRepetitionsLeft"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRepetitions"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCall"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Start"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsRunning"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Call"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Stop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Pause"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["animation"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Channel"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTargetPath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTimesArray"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValueType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetValueArray"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Load"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetTargetPath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetValueType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInterpolation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetInterpolation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Manager"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPlayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCurrentAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["StopAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlayAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCurrentAnimationId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetModel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPreviousSlice"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["Pose"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBoneIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBoneTranslationTable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetTransform"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Globalize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Localize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTransformCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Animation2"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddChannel"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAnimationSpeedFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetChannelCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["FindChannel"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetAnimationSpeedFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChannels"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["Slice"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChannelProperty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetChannelValueCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["Player"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCurrentTimeFraction"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsLooping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Advance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCurrentTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRemainingDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCurrentTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCurrentSlice"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetLooping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetPlaybackRate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["mesh"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["regex"] = {
		type = "lib",
		description = [[]],
		childs = {
			["replace"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["Result"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMatchCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPosition"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasMatch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFormat"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["jit"] = {
		type = "lib",
		description = [[]],
		childs = {
			["opt"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},

		}
	},
	["string"] = {
		type = "lib",
		description = [[]],
		childs = {
			["calc_levenshtein_similarity"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["find_longest_common_substring"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["find_similar_elements"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["is_integer"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_number"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["replace"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["split"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["join"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["remove_whitespace"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["remove_quotes"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["fill_zeroes"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["compare"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["hash"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_levenshtein_distance"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["net"] = {
		type = "lib",
		description = [[]],
		childs = {
			["RecipientFilter"] = {
				type = "class",
				description = [[]],
				childs = {
					["HasRecipient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFilterType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFilterType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRecipients"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddRecipient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveRecipient"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ClientRecipientFilter"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetRecipients"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Packet"] = {
				type = "class",
				description = [[]],
				childs = {
					["ReadPlayer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Seek"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadSoundSource"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVectori"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector4i"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WritePlayer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTimeSinceTransmission"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector4i"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToBinaryString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUniqueEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteSoundSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector2i"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteLongDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadVertex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector2i"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadQuaternion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVectori"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat2x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat2x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadFloat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Tell"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadLongDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadVector2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat2x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVertex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBinaryString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ReadChar"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadStringUntil"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteBinary"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBinary"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBool"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteChar"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadUniqueEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["pcl"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["io"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["log"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["matrix"] = {
		type = "lib",
		description = [[]],
		childs = {
			["create_perspective_matrix"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_covariance_matrix"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_look_at_matrix"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_projection_depth_bias_offset"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_from_axis_angle"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_from_axes"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_orthogonal_matrix"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["vector"] = {
		type = "lib",
		description = [[]],
		childs = {
			["random2D"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_average"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_from_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["random"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["to_min_max"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["calc_best_fitting_plane"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["calc_linear_velocity_from_angular"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_min_max"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Distance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Project"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["PlanarDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ToMatrix"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Cross"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["RotateAround"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["DotProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Equals"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Slerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetYaw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPitch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SnapToGrid"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ProjectToPlane"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetPerpendicular"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["OuterProduct"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToEulerAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__unm"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToScreenUv"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRotation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PlanarDistanceSqr"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetNormal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Normalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Length"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["LengthSqr"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["x"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["y"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["z"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["sound"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Source"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetSource"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRightStereoAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAirAbsorptionFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRelative"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOuterConeGains"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGainAuto"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["CallOnStateChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGainAuto"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRolloffFactors"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOrientation"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetAirAbsorptionFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDirectFilter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveEffect"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDirectFilter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddEffect"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetEffectParameters"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetGainRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetInnerConeAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDistanceRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGainRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSendGainAuto"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDistanceRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetTimeOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDirectGainHFAuto"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetConeAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaxGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetConeAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDirection"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStereoAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLooping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOuterConeGains"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetReferenceDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStereoAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOrientation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRolloffFactors"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPitch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPitch"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVelocity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOuterConeAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetGain"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVelocity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetEffectGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSendGainHFAuto"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRightStereoAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLeftStereoAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPaused"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Stop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRangeOffsets"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFadeInDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFadeInDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLooping"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRolloffFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMinGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTimeOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetReferenceDistance"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsStopped"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rewind"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Play"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Pause"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsRelative"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPlaying"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRolloffFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRoomRolloffFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoomRolloffFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinGain"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxGain"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetInnerConeAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOuterConeAngle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOuterConeGain"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOuterConeGain"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOuterConeGainHF"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOuterConeGainHF"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPriority"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPriority"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FadeIn"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FadeOut"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxAudibleDistance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIntensity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFadeOutDuration"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFadeOutDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDopplerFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDopplerFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLeftStereoAngle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSource"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPhonemeData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Effect"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CompressorProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["onOff"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["EqualizerProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["highGain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid2Gain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["highCutoff"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid1Width"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lowGain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lowCutoff"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid1Gain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid1Center"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid2Center"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["mid2Width"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["VocalMorpherProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["phonemeB"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["phonemeBCoarseTuning"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["phonemeACoarseTuning"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["phonemeA"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rate"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["waveform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["PitchShifterProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["coarseTune"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["fineTune"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["FrequencyShifterProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["leftDirection"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["frequency"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rightDirection"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["AutoWahProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["attackTime"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["releaseTime"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["resonance"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["peakGain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["DistortionProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["edge"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lowpassCutoff"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["gain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["eqCenter"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["eqBandwidth"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["ChorusProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["waveform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["depth"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["feedback"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["phase"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["delay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rate"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["ReverbProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["decayHFLimit"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["gain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["reflectionsDelay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["gainLF"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["density"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["diffusion"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["modulationDepth"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["gainHF"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["decayTime"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["decayHFRatio"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["decayLFRatio"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["reflectionsGain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["reflectionsPan"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lateReverbDelay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lateReverbGain"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lateReverbPan"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["echoTime"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["echoDepth"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["modulationTime"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["airAbsorptionGainHF"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["hfReference"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lfReference"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["roomRolloffFactor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["EchoProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["spread"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["feedback"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["lrDelay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["delay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["damping"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["RingModulatorProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["highpassCutoff"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["waveform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["frequency"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["FlangerProperties"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["waveform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["depth"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["feedback"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["phase"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["delay"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["rate"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},

		}
	},
	["nav"] = {
		type = "lib",
		description = [[]],
		childs = {
			["generate"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["load"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "()"
			},
			["Mesh"] = {
				type = "class",
				description = [[]],
				childs = {
					["RayCast"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetConfig"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Config"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["characterHeight"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxClimbHeight"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["walkableSlopeAngle"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["minRegionSize"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mergeRegionSize"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxEdgeLength"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["cellSize"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxSimplificationError"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["cellHeight"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["vertsPerPoly"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["sampleDetailDist"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["samplePartitionType"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["sampleDetailMaxError"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["walkableRadius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["util"] = {
		type = "lib",
		description = [[]],
		childs = {
			["splash_damage"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["is_table"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_date_time"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_faded_time_factor"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_valid"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_scale_factor"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["cubemap_to_equirectangular_texture"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["open_path_in_explorer"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["get_pretty_bytes"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_pretty_time"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["units_to_metres"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["read_scene_file"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["world_to_local"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["get_addon_path"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["open_url_in_browser"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["register_class"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["load_image"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["clamp_resolution_to_aspect_ratio"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "()"
			},
			["create_muzzle_flash"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["world_space_point_to_screen_space_uv"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["world_space_direction_to_screen_space"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["calc_screen_space_distance_to_world_space_position"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_same_object"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["depth_to_distance"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["round_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["generate_uuid_v4"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_string_hash"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["shake_screen"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["fire_bullets"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["local_to_world"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function)",
				returns = "()"
			},
			["create_giblet"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["create_explosion"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["calc_world_direction_from_2d_coordinates"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["variable_type_to_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["pack_zip_archive"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "()"
			},
			["get_class_value"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["is_valid_entity"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["capture_raytraced_screenshot"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_image_format_file_extension"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_type_name"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["fade_property"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_pretty_duration"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["remove"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["metres_to_units"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["create_particle_tracer"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["save_image"] = {
				type = "function",
				description = [[]],
				args = "(: Function, : Function, : Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_clipboard_string"] = {
				type = "function",
				description = [[]],
				args = "()",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["set_clipboard_string"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "()"
			},
			["VectoriProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector4iProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DirectoryChangeListener"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Poll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["Mat4Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ParallelJobString"] = {
				type = "class",
				inherits = "util.ParallelJob",
				description = [[]],
				childs = {
					["GetResult"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector4Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["StringProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["CharVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BoolVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["FloatVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DataBlock"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetVector4"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetChildBlocks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetKeys"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValueType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsEmpty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddBlock"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsVector4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindBlock"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetData"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetInt"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetVector2"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Vector2iProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3x2Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IntProperty"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__lt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__le"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["EntityProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat4x3Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DataStream"] = {
				type = "class",
				description = [[]],
				childs = {
					["ReadVertex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Seek"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadVector4i"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVectori"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector2i"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadQuaternion"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat4x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVectori"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat3x2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteVector4i"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToBinaryString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadMat2x3"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteQuaternion"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadMat2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat2x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteFloat"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadFloat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Tell"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteMat2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadLongDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt16"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadDouble"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteLongDouble"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadVector4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadVector2"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadStringUntil"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteAngles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat2x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat3x4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4x2"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteMat4"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVector2i"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteVertex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBinaryString"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadChar"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteString"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ReadBool"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteBinary"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadBinary"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteBool"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt64"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt64"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt8"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt8"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ReadInt32"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["WriteUInt32"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["WriteChar"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ReadUInt16"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ParallelJob"] = {
				type = "class",
				description = [[]],
				childs = {
					["Start"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetProgress"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Poll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStatus"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetResultMessage"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Cancel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsThreadActive"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Wait"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsComplete"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPending"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsCancelled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSuccessful"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector2Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ColorVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["VectorProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Version"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__lt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__le"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["minor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["revision"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["major"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Vector4Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat4x2Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat4Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector3Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat2Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat2x3Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector3iVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["LongDoubleVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector2Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["IntVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ImageBuffer"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetData"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPixelValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["IsHDRFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToHDR"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToFloat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToLDR"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearAlpha"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsFloatFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPixelValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Convert"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPixelIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetPixelOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["FlipHorizontally"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FlipVertically"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Flip"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SwapChannels"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ApplyToneMapping"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ApplyGammaCorrection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ApplyExposure"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPixelValueLDR"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetPixelCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPixelValueHDR"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["CalcLuminance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["HasAlphaChannel"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsLDRFormat"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChannelCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChannelSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPixelSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat4x3Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ParallelJobImage"] = {
				type = "class",
				inherits = "util.ParallelJob",
				description = [[]],
				childs = {
					["GetResult"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3x2Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3x4Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BoolProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Vector2iVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["FloatProperty"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__lt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__le"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Vector4iVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat2x4Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["StringVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SplashDamageInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetCone"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["origin"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["radius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["damageInfo"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["ResultCode"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMessage"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsError"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["Callback"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Call"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["CallbackHandler"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CallCallbacks"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["Path"] = {
				type = "class",
				description = [[]],
				childs = {
					["MakeRelative"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PopFront"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetComponentCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MoveUp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PopBack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFile"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Canonicalize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFileExtension"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsEmpty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveFileExtension"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBack"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPath"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToComponents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFileName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFront"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["HSVColor"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["ToRGBColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["h"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["v"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["s"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Color"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetContrastColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToVector4"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToVector"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetComplementaryColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CalcPerceivedLuminance"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Lerp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ToHSVColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToHexColorRGB"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ToHexColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["r"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["b"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["g"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["a"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BaseProperty"] = {
				type = "class",
				description = [[]],
				childs = {

				}
			},
			["ColorProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["QuaternionProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ShortVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DoubleVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["LongLongVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["QuaternionVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["EulerAnglesVector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat2Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat2x3Vector"] = {
				type = "class",
				description = [[]],
				childs = {
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__len"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PushBack"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Back"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Insert"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["At"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Erase"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Front"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Capacity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShrinkToFit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reserve"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["Size"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["EulerAnglesProperty"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__add"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__sub"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__mul"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__div"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Mat3x4Property"] = {
				type = "class",
				inherits = "util.BaseProperty",
				description = [[]],
				childs = {
					["Get"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InvokeCallbacks"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Set"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Link"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddModifier"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Unlink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["TextureInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetNormalMap"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["mipMapFilter"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["outputFormat"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["flags"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["inputFormat"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["wrapMode"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["containerFormat"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Texture"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetVkTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},

		}
	},
	["class_info_data"] = {
		type = "class",
		description = [[]],
		childs = {
			["methods"] = {
				type = "value",
				description = [[]],
				valuetype = ""
			},
			["attributes"] = {
				type = "value",
				description = [[]],
				valuetype = ""
			},
			["name"] = {
				type = "value",
				description = [[]],
				valuetype = ""
			}
		}
	},
	["table"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["function_info"] = {
		type = "lib",
		description = [[]],
		childs = {
			["get_function_name"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			},
			["get_function_overloads"] = {
				type = "function",
				description = [[]],
				args = "(: Function)",
				returns = "(: Function)",
				valuetype = "Function"
			}
		}
	},
	["shader"] = {
		type = "lib",
		description = [[]],
		childs = {
			["ShaderEntity"] = {
				type = "class",
				inherits = "shader.SceneLit3D shader.Scene3D shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["RecordDrawMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindInstanceDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["BindVertexAnimationOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoundEntity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordBindVertexAnimationOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["VertexBinding"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["inputRate"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["stride"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Graphics"] = {
				type = "class",
				inherits = "shader.Shader",
				description = [[]],
				childs = {
					["RecordDrawIndexed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindVertexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBeginDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordEndDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordBindIndexBuffer"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetRenderPass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordBindVertexBuffers"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BaseCompute"] = {
				type = "class",
				inherits = "shader.BaseComputeModule shader.Compute shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BaseGraphics"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["GameWorld"] = {
				type = "class",
				inherits = "shader.ShaderEntity shader.SceneLit3D shader.Scene3D shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["SetDepthBias"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					}
				}
			},
			["GUITextured"] = {
				type = "class",
				inherits = "shader.Graphics shader.Shader",
				description = [[]],
				childs = {

				}
			},
			["Glow"] = {
				type = "class",
				inherits = "shader.TexturedLit3D shader.ShaderEntity shader.SceneLit3D shader.Scene3D shader.Graphics shader.Shader",
				description = [[]],
				childs = {

				}
			},
			["BaseGUITextured"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.GUITextured shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Scene3D"] = {
				type = "class",
				inherits = "shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["RecordBindSceneCamera"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RecordBindRenderSettings"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ComputePipelineCreateInfo"] = {
				type = "class",
				inherits = "shader.BasePipelineCreateInfo",
				description = [[]],
				childs = {
					["AddSpecializationConstant"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					}
				}
			},
			["BaseParticle2D"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.SceneLit3D shader.Scene3D shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BaseModule"] = {
				type = "class",
				description = [[]],
				childs = {
					["InitializePipeline"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnInitialized"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnPipelinesInitialized"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShaderSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnPipelineInitialized"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPipelineCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCurrentCommandBuffer"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SceneLit3D"] = {
				type = "class",
				inherits = "shader.Scene3D shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["RecordBindLights"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["Compute"] = {
				type = "class",
				inherits = "shader.Shader",
				description = [[]],
				childs = {
					["RecordDispatch"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordBeginCompute"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordEndCompute"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordCompute"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["BaseTexturedLit3D"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.TexturedLit3D shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["BindMaterialParameters"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["InitializeGfxPipelineVertexAttributes"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InitializeGfxPipelinePushConstantRanges"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBindMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InitializeGfxPipelineDescriptorSets"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBindEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["OnBindScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBeginDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["OnEndDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DescriptorSetBinding"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["shaderStages"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["bindingIndex"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["descriptorArraySize"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["type"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BaseGraphicsModule"] = {
				type = "class",
				inherits = "shader.BaseModule",
				description = [[]],
				childs = {

				}
			},
			["BaseComputeModule"] = {
				type = "class",
				inherits = "shader.BaseModule",
				description = [[]],
				childs = {

				}
			},
			["Info"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Shader"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetEntrypointName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CreateDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPipelineInfo"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsGraphicsShader"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetGlslSourceCode"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsComputeShader"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPipelineBindPoint"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIdentifier"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSourceFilePath"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindDescriptorSets"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSourceFilePaths"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RecordBindDescriptorSet"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordPushConstants"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["BaseImageProcessing"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["ComposeRMA"] = {
				type = "class",
				inherits = "shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["ComposeRMA"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					}
				}
			},
			["BasePostProcessing"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["VertexAttribute"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["location"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["format"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["offset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BasePbr"] = {
				type = "class",
				inherits = "shader.BaseGraphicsModule shader.TexturedLit3D shader.Graphics shader.Shader shader.BaseModule",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["BindMaterialParameters"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["InitializeGfxPipelineVertexAttributes"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["InitializeGfxPipelinePushConstantRanges"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBindMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InitializeGfxPipelineDescriptorSets"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBindEntity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["OnBindScene"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnBeginDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["OnEndDraw"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DescriptorSetInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["setIndex"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["BasePipelineCreateInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["AttachPushConstantRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["AttachDescriptorSetInfo"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TexturedLit3D"] = {
				type = "class",
				inherits = "shader.GameWorld shader.ShaderEntity shader.SceneLit3D shader.Scene3D shader.Graphics shader.Shader",
				description = [[]],
				childs = {
					["RecordBindMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RecordBindClipPlane"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["GraphicsPipelineCreateInfo"] = {
				type = "class",
				inherits = "shader.BasePipelineCreateInfo",
				description = [[]],
				childs = {
					["SetAlphaToOneEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthBiasEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVertexAttributeCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDepthBoundsState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetStencilTestEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthBiasConstantFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStencilTestProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetDepthBiasClamp"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthBiasProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetCommonAlphaBlendProperties"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDepthBiasSlopeFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRasterizationProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetDepthClamp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDepthBoundsTestEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDepthBiasState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDepthBoundsTestProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetMinDepthBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFrontFace"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDepthBiasSlopeFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxDepthBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthClampEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCullMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthTestProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetColorBlendAttachmentProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDepthTestEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLogicOpState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDepthWritesEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDepthTestState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLogicOpEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSampleMask"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDynamicStates"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDynamicViewportsCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDynamicScissorBoxesCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDynamicStateEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetDepthBiasConstantFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLogicOpProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMultisamplingProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetPrimitiveRestartEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSampleCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRasterizerDiscardEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSampleMask"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMinSampleShading"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMinDepthBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBlendingProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetColorBlendAttachmentProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetSampleShadingEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSampleMaskEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetDynamicScissorBoxesCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPrimitiveTopology"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPolygonMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetViewportCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLineWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScissorBoxProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetMaxDepthBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetViewportProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["AreDepthWritesEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBlendingProperties"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDepthBiasClamp"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMultisamplingProperties"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDynamicStates"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDynamicViewportsCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScissorCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSampleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinSampleShading"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsAlphaToOneEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScissorBoxesCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetViewportsCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPrimitiveTopology"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPushConstantRanges"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRasterizationProperties"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPolygonMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCullMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrontFace"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLineWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSampleShadingState"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScissorBoxProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddSpecializationConstant"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetStencilTestProperties"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubpassId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertexAttributeProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetViewportProperties"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsAlphaToCoverageEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsDepthClampEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsPrimitiveRestartEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsRasterizerDiscardEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsSampleMaskEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AttachVertexAttribute"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetAlphaToCoverageEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},

		}
	},
	["gui"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Element"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetAnchor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEndPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveStyleClass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["EnableThinking"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetClippingEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsClippingEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InjectCharInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["CenterToParentY"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindDescendantByName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Draw"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMouseInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScale"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetAlwaysUpdate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Think"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DisableThinking"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBackgroundElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAttachmentPosProperty"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBackgroundElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindDescendantsByName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetBottom"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InjectKeyPress"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ApplySkin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRemoveOnParentRemoval"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAutoSizeToContents"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasAnchor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetVisible"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMouseInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTop"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnchor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetThinkingEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAttachmentPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetAttachmentPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnchorLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAbsoluteAttachmentPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDescendantOf"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["KillFocus"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetKeyboardInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAnchorRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnchorTop"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAnchorBottom"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetHalfHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCenterY"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearAnchor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHalfSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRootWindow"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCenterX"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHalfWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTooltip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScaleProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCenterPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsCursorInBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CenterToParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearStyleClasses"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CenterToParentX"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsUpdateScheduled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetZPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindChildIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRootElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsVisible"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChildren"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsAncestorOf"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetZPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetKeyboardInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasFocus"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RequestFocus"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["TrapFocus"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsHidden"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsParentVisible"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScrollInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetScrollInputEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCursorMovementCheckEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCursorMovementCheckEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAbsolutePos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAbsolutePos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetColorProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFocusProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVisibilityProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPosProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSizeProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFadingIn"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMouseInBoundsProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetColorRGB"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAlpha"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAlpha"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Update"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Wrap"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFirstChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChild"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsPosInBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCursorPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetX"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetY"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetX"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetY"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindChildByName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InjectMouseInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SizeToContents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["FadeIn"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["FadeOut"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsFading"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetStyleClasses"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFadingOut"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["InjectMouseMoveInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InjectMouseClick"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["InjectKeyboardInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["InjectScrollInput"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsValid"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Remove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsFocusTrapped"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetClass"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveSafely"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetScale"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddAttachment"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["CallCallbacks"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["GetAutoAlignToParentY"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Resize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ClearParent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDescendant"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsAncestor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["FindChildrenByName"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAutoAlignToParent"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAutoAlignToParentX"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSkin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ScheduleUpdate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ResetSkin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddStyleClass"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCursor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCursor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveElementOnRemoval"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTooltip"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["HasTooltip"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLeft"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBottom"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["impl"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["Frame"] = {
				type = "class",
				inherits = "gui.Transformable gui.Element",
				description = [[]],
				childs = {
					["SetTitle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTitle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCloseButtonEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["TextEntry"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetText"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTextElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsNumeric"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsEditable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEditable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaxLength"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxLength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSelectable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsMultiLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMultiLine"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsSelectable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCaretElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Text"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTextLength"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAutoBreakMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetText"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["MoveText"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetTagsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["PopFrontLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetLine"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsShadowEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFont"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShadowXOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["EnableShadow"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShadowXOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShadowColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTagArgument"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["SetShadowOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetShadowYOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShadowColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShadowOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShadowYOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShadowAlpha"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShadowAlpha"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShadowBlurSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetShadowBlurSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AreTagsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["PopBackLine"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveText"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["RemoveLine"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InsertText"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["AppendText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AppendLine"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Substr"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetLineCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DebugSSAO"] = {
				type = "class",
				inherits = "gui.TexturedShape gui.Shape gui.Element",
				description = [[]],
				childs = {
					["SetUseBlurredSSAOImage"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Shape"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["InvertVertexPositions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearVertices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertexPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddVertex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Arrow"] = {
				type = "class",
				inherits = "gui.Shape gui.Element",
				description = [[]],
				childs = {
					["SetDirection"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Icon"] = {
				type = "class",
				inherits = "gui.TexturedShape gui.Shape gui.Element",
				description = [[]],
				childs = {
					["SetClipping"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Base"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["OnInitialize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CharCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnRemove"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnThink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnDraw"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["MouseCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["KeyboardCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["ScrollCallback"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnUpdate"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnVisibilityChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnSizeChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["OnColorChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["OnAlphaChanged"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["OnCursorEntered"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnCursorExited"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnFocusGained"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnFocusKilled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OnInitialThink"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Container"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetPadding"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPaddingTop"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPaddingBottom"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPaddingRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetPaddingLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetPadding"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPaddingTop"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPaddingRight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPaddingBottom"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetPaddingLeft"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Console"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["GetCommandLineEntryElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTextLogElement"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AppendText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxLogLineCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrame"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetText"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxLogLineCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["SilkIcon"] = {
				type = "class",
				inherits = "gui.Icon gui.TexturedShape gui.Shape gui.Element",
				description = [[]],
				childs = {
					["SetIcon"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["TexturedShape"] = {
				type = "class",
				inherits = "gui.Shape gui.Element",
				description = [[]],
				childs = {
					["GetAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetShader"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAlphaCutoff"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetChannelSwizzle"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetAlphaMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["InvertVertexUVCoordinates"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetChannelSwizzle"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertexUVCoord"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SizeToTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTexture"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetAlphaCutoff"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMaterial"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaterial"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Checkbox"] = {
				type = "class",
				inherits = "gui.Shape gui.Element",
				description = [[]],
				childs = {
					["SetChecked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsChecked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Toggle"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["DebugDepthTexture"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetContrastFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetContrastFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTexture"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["DebugShadowMap"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetContrastFactor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetContrastFactor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetShadowMapSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetLightSource"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ProgressBar"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["GetValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetProgress"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRange"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetProgress"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetPostFix"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddOption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOptions"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["Table"] = {
				type = "class",
				inherits = "gui.Container gui.Element",
				description = [[]],
				childs = {
					["GetRowCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSelectableMode"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Clear"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRowHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRowHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSelectableMode"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddRow"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetColumnWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["AddHeaderRow"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSortable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsSortable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScrollable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsScrollable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRow"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSelectedRows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRows"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveRow"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MoveRow"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SelectRow"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFirstSelectedRow"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Cell"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["GetFirstElement"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetColSpan"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetColSpan"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Row"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["Select"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["IsSelected"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["InsertElement"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Deselect"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCellCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetCellWidth"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetCell"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRowIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Slider"] = {
				type = "class",
				inherits = "gui.ProgressBar gui.Element",
				description = [[]],
				childs = {
					["IsBeingDragged"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["SnapArea"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["GetTriggerArea"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["TreeList"] = {
				type = "class",
				inherits = "gui.Table gui.Element",
				description = [[]],
				childs = {
					["AddItem"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["ExpandAll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["CollapseAll"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetRootItem"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Element"] = {
						type = "class",
						inherits = "gui.Table.Row gui.Element",
						description = [[]],
						childs = {
							["AddItem"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Expand"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetItems"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Collapse"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},
					["Row"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["SetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["InsertElement"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Deselect"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCellCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsSelected"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetCellWidth"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetCell"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRowIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Select"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Cell"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["GetFirstElement"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetColSpan"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetColSpan"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},

				}
			},
			["ScrollContainer"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetAutoStickToBottom"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsContentsHeightFixed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ShouldAutoStickToBottom"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsContentsWidthFixed"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetContentsWidthFixed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetContentsHeightFixed"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetScrollAmountY"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetVerticalScrollBar"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetHorizontalScrollBar"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScrollAmount"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetScrollAmountX"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["OutlinedRect"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetOutlineWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOutlineWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Line"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetStartPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStartPos"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEndPos"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetStartColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStartPosProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEndColor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetStartColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEndColor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEndPosProperty"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetLineWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetLineWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Transformable"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["Close"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDraggable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetDragArea"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetDragBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaxHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetResizable"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsDraggable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMinSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetMinHeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsResizable"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMinWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinHeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsBeingDragged"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxWidth"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMaxWidth"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetDragBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsResizeRatioLocked"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSnapTarget"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBeingResized"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMaxSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMinSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetResizeRatioLocked"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ScrollBar"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetScrollAmount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetUp"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetScrollAmount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddScrollOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetScrollOffset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsHorizontal"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetScrollOffset"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetHorizontal"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsVertical"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["CommandLineEntry"] = {
				type = "class",
				inherits = "gui.TextEntry gui.Element",
				description = [[]],
				childs = {
					["SetAutoCompleteEntryLimit"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetAutoCompleteEntryLimit"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["RoundedRect"] = {
				type = "class",
				inherits = "gui.Shape gui.Element",
				description = [[]],
				childs = {
					["GetRoundness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundTopRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsTopRightRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCornerSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsTopLeftRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundBottomLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCornerSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundTopLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRoundBottomRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBottomLeftRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsBottomRightRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["DropDownMenu"] = {
				type = "class",
				inherits = "gui.TextEntry gui.Element",
				description = [[]],
				childs = {
					["SetListItemCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOptionValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddOption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CloseMenu"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ScrollToOption"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SelectOption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearOptions"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SelectOptionByText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetOptionText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetOptionText"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetOptionValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetOptionCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["OpenMenu"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["HasOption"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToggleMenu"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOptionElement"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsMenuOpen"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSelectedOption"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearSelectedOption"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["FindOptionSelectedByCursor"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["RoundedTexturedRect"] = {
				type = "class",
				inherits = "gui.TexturedShape gui.Shape gui.Element",
				description = [[]],
				childs = {
					["GetRoundness"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundTopRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsTopRightRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundness"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCornerSize"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsTopLeftRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundBottomLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCornerSize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetRoundTopLeft"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetRoundBottomRight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["IsBottomLeftRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IsBottomRightRound"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["NumericTextEntry"] = {
				type = "class",
				inherits = "gui.TextEntry gui.Element",
				description = [[]],
				childs = {
					["SetRange"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetMaxValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMinValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMaxValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMinValue"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["GridPanel"] = {
				type = "class",
				inherits = "gui.Table gui.Element",
				description = [[]],
				childs = {
					["GetColumnCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddItem"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["Cell"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["GetFirstElement"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetColSpan"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetColSpan"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["Row"] = {
						type = "class",
						inherits = "gui.Container gui.Element",
						description = [[]],
						childs = {
							["IsSelected"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetCellWidth"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["InsertElement"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetCellCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCell"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetRowIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Select"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Deselect"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							}
						}
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["Button"] = {
				type = "class",
				inherits = "gui.Element",
				description = [[]],
				childs = {
					["SetText"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetText"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["DrawInfo"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetColor"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetPostTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["__init"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["transform"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["useScissor"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["commandBuffer"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["size"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["offset"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},

		}
	},
	["vrp"] = {
		type = "lib",
		description = [[]],
		childs = {
			["Version"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__le"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__lt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ToString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["minor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["revision"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["major"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["pfm"] = {
		type = "lib",
		description = [[]],
		childs = {
			["math"] = {
				type = "lib",
				description = [[]],
				childs = {

				}
			},
			["impl"] = {
				type = "lib",
				description = [[]],
				childs = {
					["projects"] = {
						type = "lib",
						description = [[]],
						childs = {

						}
					},

				}
			},
			["Version"] = {
				type = "class",
				description = [[]],
				childs = {
					["ToString"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__lt"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__le"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reset"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["major"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["minor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["revision"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
	["sfm"] = {
		type = "lib",
		description = [[]],
		childs = {

		}
	},
	["Model"] = {
		type = "lib",
		description = [[]],
		childs = {
			["VertexAnimation"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetMeshAnimations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MeshAnimation"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetMesh"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFrames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Frame"] = {
								type = "class",
								description = [[]],
								childs = {
									["GetFlags"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["Rotate"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetFlags"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexPosition"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetVertexCount"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetVertices"] = {
										type = "function",
										description = [[]],
										args = "()",
										returns = "()"
									},
									["GetDeltaValue"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexCount"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["GetVertexPosition"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetVertexNormal"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									},
									["GetVertexNormal"] = {
										type = "function",
										description = [[]],
										args = "(: Function)",
										returns = "()"
									},
									["SetDeltaValue"] = {
										type = "function",
										description = [[]],
										args = "(: Function, : Function)",
										returns = "()"
									}
								}
							},

						}
					},

				}
			},
			["MeshGroup"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMeshCount"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetMeshes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					}
				}
			},
			["ExportInfo"] = {
				type = "class",
				description = [[]],
				childs = {
					["SetAnimationList"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["aoSamples"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["exportAnimations"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["aoResolution"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["scale"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["imageFormat"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["mergeMeshesByMaterial"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["aoDevice"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["exportSkinnedMeshData"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["exportMorphTargets"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["exportImages"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["embedAnimations"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["fullExport"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["normalizeTextureNames"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["enableExtendedDDS"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["saveAsBinary"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["verbose"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["generateAo"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Flex"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetOperations"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertexAnimation"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFrameIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertexAnimation"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Operation"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["index"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["value"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							},
							["type"] = {
								type = "value",
								description = [[]],
								valuetype = ""
							}
						}
					},

				}
			},
			["CollisionMesh"] = {
				type = "class",
				description = [[]],
				childs = {
					["IsSoftBody"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddTriangle"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetVertexCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetConvex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneParentId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetShape"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetOrigin"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetAABB"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSurfaceMaterialIds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["IntersectAABB"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["IsConvex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSoftBodyMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetAABB"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetBoneParentId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Centralize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetTriangles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetOrigin"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSurfaceMaterialId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Update"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddVertex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTriangles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetSoftBodyTriangles"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClipAgainstPlane"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["ClearSoftBodyAnchors"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVolume"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearVertices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSoftBodyAnchor"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetVolume"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearTriangles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetVertices"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSoftBody"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSoftBodyMesh"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftBodyInfo"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSoftBodyTriangles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["RemoveSoftBodyAnchor"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetSoftBodyAnchors"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSurfaceMaterialId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					}
				}
			},
			["Animation"] = {
				type = "class",
				description = [[]],
				childs = {
					["ClearFrames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEventArgs"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetFrames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetBoneList"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Reverse"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActivityName"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetBoneWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetFadeInTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddBoneId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBoneWeights"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFadeOutTime"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddFrame"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["CalcRenderBounds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SaveLegacy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetFadeOutTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneList"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["LookupBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["GetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrame"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetActivity"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetActivityWeight"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFlags"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetActivityWeight"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFPS"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetActivity"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBlendController"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFadeInTime"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEvents"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetFPS"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveFlags"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddEvent"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetDuration"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetEventCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetFrameCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetEventData"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function, : Function)",
						returns = "()"
					},
					["SetEventType"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["Frame"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetBoneMatrix"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBoneTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBoneRotation"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetMoveTranslation"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Scale"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBonePosition"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Localize"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Globalize"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["CalcRenderBounds"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetBoneScale"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetMoveTranslationX"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetMoveTranslationZ"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetFlexControllerWeights"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetMoveTranslation"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetFlexControllerWeights"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetMoveTranslationX"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBoneTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetBoneRotation"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetMoveTranslationZ"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBoneScale"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetLocalBoneTransform"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetBoneCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetBonePose"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetBonePose"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["Copy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetFlexControllerIds"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBoneCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Translate"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							}
						}
					},

				}
			},
			["Mesh"] = {
				type = "class",
				description = [[]],
				childs = {
					["RemoveSubMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Update"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Centralize"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetVertexCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Scale"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["ClearSubMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubMeshes"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCenter"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddSubMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetSubMeshes"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetCenter"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetTriangleCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubMeshCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetSubMesh"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetReferenceId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Rotate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBounds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetTriangleVertexCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Translate"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Sub"] = {
						type = "class",
						description = [[]],
						childs = {
							["ReserveVertices"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVertexCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["FlipTriangleWindingOrder"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetVertexNormal"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["SetVertexUV"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["AddVertex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearExtendedVertexWeights"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddPoint"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertexWeights"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Rotate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVertexPosition"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetVertexNormal"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetPose"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexPosition"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetSkinTextureIndex"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetIndexCount"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTriangleVertexCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ReserveTriangles"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetTriangleCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetCenter"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetPose"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetUVSetNames"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetVertices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddTriangle"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function)",
								returns = "()"
							},
							["GetNormals"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetReferenceId"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetSkinTextureIndex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVertexAlpha"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["Transform"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetBounds"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetVertex"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetVertex"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearVertexWeights"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexUV"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertexAlpha"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertexBuffer"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetVertexWeight"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Optimize"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GenerateNormals"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ReserveVertexWeights"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["NormalizeUVCoordinates"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ApplyUVMapping"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function, : Function, : Function, : Function, : Function, : Function, : Function)",
								returns = "()"
							},
							["SetVertexTangent"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetGeometryType"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetGeometryType"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetIndexBuffer"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["AddLine"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["ClearAlphas"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearUVSets"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["HasUVSet"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["ClearVertexData"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTangents"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Translate"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Scale"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetVertexWeightBuffer"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetAlphaBuffer"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetSceneMesh"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["Update"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetTriangles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClipAgainstPlane"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["ClearVertices"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearTriangles"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["GetBiTangents"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__eq"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["SetVertexWeight"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetUVs"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["AddUVSet"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["Copy"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							}
						}
					},

				}
			},
			["VertexWeight"] = {
				type = "class",
				description = [[]],
				childs = {
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["boneIds"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["weights"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Skeleton"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetRootBones"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBones"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["AddBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["LookupBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["MakeRootBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBoneCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["ClearBones"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetBoneHierarchy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["IsRootBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Merge"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetBone"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["Bone"] = {
						type = "class",
						description = [[]],
						childs = {
							["GetParent"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["__tostring"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["ClearParent"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetParent"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetName"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["SetName"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetChildren"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							},
							["IsAncestorOf"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["IsDescendantOf"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["GetID"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "()"
							}
						}
					},

				}
			},
			["Joint"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetKeyValues"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetType"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetCollisionMeshId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetParentCollisionMeshId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetType"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["SetCollisionsEnabled"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetKeyValues"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["RemoveKeyValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetArgs"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetKeyValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function)",
						returns = "()"
					},
					["SetArgs"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetChildBoneId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetParentBoneId"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["GetCollisionsEnabled"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["collide"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["Eyeball"] = {
				type = "class",
				description = [[]],
				childs = {
					["GetUpperLidFlexIndices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetUpperLidFlexAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetLowerLidFlexIndices"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetLowerLidFlexAngles"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetLowerLidFlexIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetUpperLidFlexIndex"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["origin"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["radius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["boneIndex"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["zOffset"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["forward"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["up"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["irisMaterialIndex"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["maxDilationFactor"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["irisUvRadius"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["irisScale"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["name"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},
			["FlexAnimation"] = {
				type = "class",
				description = [[]],
				childs = {
					["LookupLocalFlexControllerIndex"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFlexControllerCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SaveLegacy"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["AddFrame"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["ClearFrames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["Save"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["SetFlexControllerValue"] = {
						type = "function",
						description = [[]],
						args = "(: Function, : Function, : Function)",
						returns = "()"
					},
					["GetFps"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["RemoveFrame"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFrameCount"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFrame"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetFrames"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetFps"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["GetFlexControllerIds"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["SetFlexControllerIds"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["AddFlexControllerId"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["Frame"] = {
						type = "class",
						description = [[]],
						childs = {
							["SetFlexControllerValues"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFlexControllerValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function)",
								returns = "()"
							},
							["GetFlexControllerValueCount"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							},
							["SetFlexControllerValue"] = {
								type = "function",
								description = [[]],
								args = "(: Function, : Function)",
								returns = "()"
							},
							["GetFlexControllerValues"] = {
								type = "function",
								description = [[]],
								args = "()",
								returns = "(: Function)",
								valuetype = "Function"
							}
						}
					},

				}
			},
			["Vertex"] = {
				type = "class",
				description = [[]],
				childs = {
					["Copy"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["GetBiTangent"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "(: Function)",
						valuetype = "Function"
					},
					["__tostring"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["__eq"] = {
						type = "function",
						description = [[]],
						args = "(: Function)",
						returns = "()"
					},
					["__init"] = {
						type = "function",
						description = [[]],
						args = "()",
						returns = "()"
					},
					["position"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["uv"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["tangent"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					},
					["normal"] = {
						type = "value",
						description = [[]],
						valuetype = ""
					}
				}
			},

		}
	},
}