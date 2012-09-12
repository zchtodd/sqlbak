import unittest
import subprocess 
import fcntl
import time
import os

class TestSQLBak(unittest.TestCase):
    
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def sqlbak(self, args):
        x = subprocess.Popen(["sqlbak"] + args,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

        output = x.communicate()[0]
        assert "Segmentation" not in output

        return output

    def test_usage_01(self):
        """Usage message on empty invocation."""

        usage = self.sqlbak([])

        assert "usage: sqlbak directory" in usage
        assert "[--tarname=/path/to/tar]" in usage
        assert "[--ms-towait=ms]" in usage
        assert "[--dry-run]" in usage
        assert "[--integrity-check]" in usage
        assert "[--follow-links]" in usage
        assert "[--verbose]" in usage
        assert "[--version]" in usage

    def test_usage_02(self):
        """Usage message on improper invocation."""

        usage = self.sqlbak(["--foobaz"])

        assert "usage: sqlbak directory" in usage
        assert "[--tarname=/path/to/tar]" in usage
        assert "[--ms-towait=ms]" in usage
        assert "[--dry-run]" in usage
        assert "[--integrity-check]" in usage
        assert "[--follow-links]" in usage
        assert "[--verbose]" in usage
        assert "[--version]" in usage

    def test_version_01(self):
        """Display version number on --version invocation."""

        version = self.sqlbak(["--version"])
        self.assertTrue("sqlbak v" in version)
 
    def test_locked_file_01(self):
        """Back up a locked database file.  Release lock before
           timeout occurs in the backup function."""
        
        f = open("tests/locked.db3", "a+")
        fcntl.lockf(f.fileno(), fcntl.LOCK_EX)     
 
        x = subprocess.Popen(["sqlbak", "tests"],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

        time.sleep(1)
        fcntl.lockf(f.fileno(), fcntl.LOCK_UN)
        f.close()

        result = x.communicate()[0]

        self.assertTrue("cannot obtain lock" not in result)

    def test_locked_file_02(self):
        """Back up a locked database file.  Hold lock past timeout
           threshold."""
        
        f = open("tests/locked.db3", "a+")
        fcntl.lockf(f.fileno(), fcntl.LOCK_EX)     
 
        x = subprocess.Popen(["sqlbak", "tests"],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

        time.sleep(3)
        fcntl.lockf(f.fileno(), fcntl.LOCK_UN)
        f.close()

        result = x.communicate()[0]

        self.assertTrue("cannot obtain lock" in result)
        self.assertTrue("lock held by pid %d" % os.getpid() in result)

    def test_locked_file_03(self):
        """Back up a locked database file.  Hold lock past default 
           timeout threshold but release before specified timeout."""
        
        f = open("tests/locked.db3", "a+")
        fcntl.lockf(f.fileno(), fcntl.LOCK_EX)     
 
        x = subprocess.Popen(["sqlbak", "tests", "--ms-towait=4000"],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)

        time.sleep(3)
        fcntl.lockf(f.fileno(), fcntl.LOCK_UN)
        f.close()

        result = x.communicate()[0]

        self.assertTrue("cannot obtain lock" not in result)

    def test_tarname_opt_01(self):
        """Specify a tarname."""

        result = self.sqlbak([".", "--tarname=test.tar.gz"])
        self.assertTrue(os.path.exists("test.tar.gz"))

if __name__ == "__main__":
    unittest.main() 
