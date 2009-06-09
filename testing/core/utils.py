import unittest, subprocess, os

class FirtreeTestCase(unittest.TestCase):
    def assertCairoSurfaceMatches(self, surface, tag):
        out_name = 'outputs/%s.png' % tag
        exp_name = 'expected/%s.png' % tag
        surface.write_to_png(out_name)
        self.assert_(images_match(out_name, exp_name))

    def assertPixbufMatches(self, pb, tag):
        out_name = 'outputs/%s.png' % tag
        exp_name = 'expected/%s.png' % tag
        pb.save(out_name, 'png')
        self.assert_(images_match(out_name, exp_name))

    def assertKernelCompiled(self, k):
        if(not k.get_compile_status()):
            print("Kernel expected to compile but didn't. Log:")
            print(k.get_compile_log())
        self.assert_(k.get_compile_status())

def images_match(im1, im2):
    # FIXME: This uses hard coded paths :(
    diff_prog = '/usr/bin/perceptualdiff'

    if(not os.path.exists(diff_prog)):
        raise RuntimeError('perceptual diff program not found at (%s).' % diff_prog)

    if(not os.path.exists(im1)):
        raise RuntimeError('Could not find image %s for comparison.' % im1)

    if(not os.path.exists(im2)):
        raise RuntimeError('Could not find image %s for comparison.' % im2)

    pass_flag = subprocess.call((diff_prog, im1, im2), 
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    return (pass_flag != 0)
    

# vim:sw=4:ts=4:et:autoindent
