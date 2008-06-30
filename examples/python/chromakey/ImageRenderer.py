
import gtk			# Calls gtk module
import gtk.glade		# Calls gtk.glade module
import gtk.gtkgl		# Calls gtk.gtkgl module
import gobject

from OpenGL.GL import *
from OpenGL.GLU import *

import Firtree

class ImageRenderer(gtk.DrawingArea, gtk.gtkgl.Widget):
	def __init__(self):
		gtk.DrawingArea.__init__(self)

		display_mode = (gtk.gdkgl.MODE_RGB | gtk.gdkgl.MODE_DOUBLE)
		glconfig = gtk.gdkgl.Config(mode = display_mode)

		self.set_gl_capability(glconfig)
		self.connect('expose_event', self.on_expose_event)
		self.connect('configure_event', self.on_configure_event)
		self.connect('button_press_event', self.on_button_press_event)
		self.connect('button_release_event', self.on_button_release_event)
		self.connect('scroll_event', self.on_scroll_event)
		self.connect('motion_notify_event', self.on_motion_notify_event)
		self.connect('realize', self.on_realize)

		self.add_events(gtk.gdk.BUTTON_PRESS_MASK)
		self.add_events(gtk.gdk.BUTTON_RELEASE_MASK)
		self.add_events(gtk.gdk.SCROLL_MASK)
		self.add_events(gtk.gdk.POINTER_MOTION_MASK)
		self.add_events(gtk.gdk.POINTER_MOTION_HINT_MASK)

		self._image = None
		self._context = None
		self._realized = False

		self._zoom = 1.0

		self._isDragging = False
		self._dragAnchor = (0, 0, 0)

		# Pre zoom offset
		self._offset = None

		self._vpSize = (0, 0)

	def refresh(self):
		self.window.invalidate_rect(self.allocation, False)

	def set_image(self, image):
		self._image = image
		self._sampler = None

		self.refresh()

	def on_scroll_event(self, widget, event):
		if(event.direction == gtk.gdk.SCROLL_UP):
			self._zoom *= 0.9
		elif(event.direction == gtk.gdk.SCROLL_DOWN):
			self._zoom *= 1.1

		self.refresh()
	
	def on_realize(self, widget):
		self._realized = True

	def on_motion_notify_event(self, widget, event):
		if(not self._isDragging):
			return

		pointerPos = self.window.get_pointer()
		delta = (pointerPos[0] - self._dragAnchor[0], pointerPos[1] - self._dragAnchor[1])

		self._offset = (self._dragOffset[0] - self._zoom*delta[0], self._dragOffset[1] + self._zoom*delta[1])

		self.refresh()

	def on_button_release_event(self, widget, event):
		self._isDragging = False

		self.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.ARROW))

	def on_button_press_event(self, widget, event):
		self._isDragging = True
		self._dragAnchor = self.window.get_pointer()
		self._dragOffset = self._offset

		self.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.HAND1))

	def on_configure_event(self, widget, event):
		if(not self._realized):
			return
		
		if(self._offset == None):
			self._offset = (0.5 * self._zoom * event.width, 0.5 * self._zoom * event.height)

		gldrawable = self.get_gl_drawable()
		glcontext = self.get_gl_context()
		gldrawable.gl_begin(glcontext) 

		self._vpSize = (event.width, event.height)

		if(self._context == None):
			self._context = Firtree.OpenGLRenderingContext.Create()
		
		glViewport(0, 0, self._vpSize[0], self._vpSize[1])
		glClearColor(0,0,0,1)

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		glOrtho(0.0, self._vpSize[0], 0.0, self._vpSize[1], -1.0, 1.0)
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()

	def on_expose_event(self, *args):
		gldrawable = self.get_gl_drawable()
		glcontext = self.get_gl_context()
		gldrawable.gl_begin(glcontext)      
		self.display()
		gldrawable.swap_buffers()
		gldrawable.gl_end()
		
	def display(self):
		glClear(GL_COLOR_BUFFER_BIT)

		botLeftX = self._offset[0] - 0.5*self._zoom*self._vpSize[0]
		botLeftY = self._offset[1] - 0.5*self._zoom*self._vpSize[1]
		topRightX = self._offset[0] + 0.5*self._zoom*self._vpSize[0]
		topRightY = self._offset[1] + 0.5*self._zoom*self._vpSize[1]

		# Calculate the source rect after offset and zoom
		srcRect = Firtree.Rect2D(
			Firtree.Point2D(botLeftX, botLeftY),
			Firtree.Size2D(self._zoom * self._vpSize[0], self._zoom * self._vpSize[1]))
			
		dstRect = Firtree.Rect2D(0, 0, self._vpSize[0], self._vpSize[1])

		if((self._image != None) and (self._context != None)):
			# Render the image into the framebuffer.
			self._context.RenderInRect(self._image, 
				dstRect, srcRect)



# vim:sw=4:ts=4:autoindent