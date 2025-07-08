#!/usr/bin/python3
import os.path
import re
from io import BytesIO
from subprocess import check_output


def simple_call(cmd):
	return check_output(cmd.split(" "))


file = "/Users/me/dev/apps/wasp/source/Config.h"
if not os.path.exists(file): raise Exception("no file")

print("update_version in ",file)

class IncrementSemanticVersion():

	def _new_version(self, version):
		major, minor, patch = [int(i) for i in version.split(".")]
		return "{}.{}.{}".format(major, minor, patch + 1)


	def _update_version(self):
		pattern = re.compile('^(static chars wasp_version)\\s*=\\s*"([0-9\\.]+)";')
		output = BytesIO()
		new_version=None
		count=0
		with open(file, "rt") as fp:
			for line in fp:
				count=count+1
				result = pattern.match(line) 
				if not result:
					output.write(line.encode())
				else:
					wasp_chars, version = result.groups()
					print("old version",version)
					new_version = self._new_version(version)
					output.write('{} = "{}";\n'.format(wasp_chars, new_version).encode())

		if not new_version:
			raise Exception("NO new_version")

		# sys.setdefaultencoding('UTF8')
		# override file, hopefully all is ok
		with open(file, "wt") as fp:
			fp.write(str(output.getvalue(), 'UTF-8'))
		return new_version

	def _run(self):
		self.major = False
		self.minor = False
		# if simple_call("git status --porcelain").strip():
		# 	os.system("git commit -a --allow-empty-message -m ''  ")
		# raise DistutilsError("Uncommited changes, commit all changes before release")

		new_version = self._update_version()
		print("new_version "+new_version)
		# self.update_init(new_version)
		# self.distribution.metadata.version = new_version
		# os.system("git commit -m Release {}".format(new_version))
		# os.system("git tag release-{}".format(new_version))


IncrementSemanticVersion()._run()
