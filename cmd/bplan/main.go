package main

import "github.com/AllenDang/giu"

const buttonW, buttonH = 200, 80

func loop() {
	giu.SingleWindow().Layout(
		giu.Custom(func() {
			availableW, availableH := giu.GetAvailableRegion()
			itemSpacingX, itemSpacingY := giu.GetItemSpacing()
			giu.Layout{
				giu.Dummy(0, max(availableH/2-buttonH/2-itemSpacingY, 0)),
				giu.Row(
					giu.Dummy(max(availableW/2-buttonW/2-itemSpacingX, 0), 0),
					giu.Button("I'm a totally centered button").Size(buttonW, buttonH),
				),
				// Center Horizontally (experimental!) alternative
				//giu.Align(giu.AlignCenter).To(giu.Button("I'm a centered button").Size(buttonW, buttonH)),
			}.Build()
		}),
	)
}

func main() {
	wnd := giu.NewMasterWindow("Vertical+Horizontal align [discussion #529]", 640, 480, 0)
	wnd.Run(loop)
}
