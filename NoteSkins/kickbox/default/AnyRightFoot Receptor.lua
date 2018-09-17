local t =
	Def.ActorFrame {
	Def.Sprite {
		Texture = "_AnyRightFoot receptor underlay",
		Frame0000 = 0,
		Delay0000 = 1,
		InitCommand = NOTESKIN:GetMetricA("ReceptorArrow", "InitCommand"),
		NoneCommand = NOTESKIN:GetMetricA("ReceptorArrow", "NoneCommand")
	},
	Def.Sprite {
		Texture = "_AnyRightFoot receptor frame",
		Frame0000 = 0,
		Delay0000 = 1,
		InitCommand = NOTESKIN:GetMetricA("ReceptorOverlay", "InitCommand"),
		PressCommand = NOTESKIN:GetMetricA("ReceptorOverlay", "PressCommand"),
		LiftCommand = NOTESKIN:GetMetricA("ReceptorOverlay", "LiftCommand"),
		NoneCommand = NOTESKIN:GetMetricA("ReceptorArrow", "NoneCommand")
	}
}
return t
