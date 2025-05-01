# UI VDEV

I want to rewrite the UI VDEV to be a lot simpler, generic, and smaller.
Here are a few ideas I have.

## Write it in Go

Aside from being a much simpler language which I'll probably enjoy writing the large amount of code required for the UI VDEV in, since VDEVs will use the same KOS/library as regular apps and I want to focus on being able to write these apps in C and Go initially, I think one of the two is the best choice.
This does mean I need to first write a Go library.

## Semantics

Previously, the UI VDEV provided quite a few different element types for different semantic elements (e.g. paragraph, title, subtitle, log, etc, which are all just text elements behind the scenes).
I think a much more elegant and scalable solution is for there to be only a few super generic elements (e.g. div, title, text, text input) and then to be able to add a semantic tag to them (e.g. button, link, title, subtitle, etc).
With this semantic tag, the device can then decide which styles it wants to apply to the element, depending on what platform its running on e.g.

TODO Should buttons be semantics or elements? Because we could want to be able to set an icon on a button.
But then again if we want to be able to set an icon we might want to be able to set much more complex things.
So I'm leaning towards a no here.

### Accessibility

This could actually help a lot with accessibility.
If we force a semantic tag to be used for everything, it will force people to define exactly what their div is semantically (e.g. main content, navbar, etc).

## Backends

I don't want to focus on a FB backend anymore, because I think it'd be anyway nice to ship the installer with DRM at minimum, and then maybe ship Mesa with just swrast.
This will probably be faster than actually rolling my own FB backend.

Either way, I want very little backend creation and management to be done in the UI VDEV.
There's really two things the VDEV needs:

- To store backend- and element-specific data for each element. E.g. a WebGPU texture for the text.
- To be able to render the UI to the screen. E.g. by getting a WebGPU command encoder and adding a render pass to it for all UI elements.

So the VDEV needs a backend context object which can store the backend-specific data for each element across renders.
Stuff like text rendering will be done on the fly before rendering if needed, so previous UI steps don't actually need the backend context (TODO except the layout step will need information about how big text is rendered actually).

### Async

We naturally need a way to render the UI asynchronously.
Since the rendering step is the only step that will actually take time, I think its best to just support thread-safety for the rendering step and let the app asynchronously run a main loop independent of the window loop or whatever.
E.g., there could be a lock for all UI changing calls, then when the backend renders, it copied the current UI state (UI state locked) and renders it (UI state unlocked for the whole duration of this, but render lock/backend state locked):

```c
void* button = ui_add_text(ui_state, "Click me!", UI_SEMANTICS_BUTTON);

void update_loop(void) {
	sleep(1); // Big computation.

	if (ui_pressed(button)) { // Check if the button was pressed and clear it in the state. Button will render as pressed in the meantime. Or maybe a loading spinner if long enough?
		ui_add_text(ui_state, "test", UI_SEMANTICS_PARAGRAPH); // Changes UI state, can be done even when maybe rendering, not a real function.
	}
}

void redraw(void) {
	WGPUCommandBuffer command_buffer;
	ui_backend_render(ui_backend_state, command_buffer);
}

run_async(update_loop); // Should (try to) run at the frame rate of the device at minimum.
win_register_redraw_cb(win, redraw);
win_loop(win);
```

### Other systems

I could imagine having backends for other UI systems so apps feel more native on those platforms.
E.g. having an AppKit/UIKit backend for macOS/iOS, Jetpack Compose for Android, or a libadwaita backend for GNOME.

## Styling

I want styling to be done 100% generically.
Note: I don't consider layout to be styling.
Only stuff like:

- Background/text colour.
- Rounded corners.
- Borders.
- Shadows.
- Blurred backgrounds.
- Animations.

## Layout engine

I did think of using [Clay](https://github.com/nicbarker/clay), which is new and seems pretty nice, but I'd have to write Go bindings and I don't know that it's super necessary for something relatively simple like this.
And I think its better suited 
