# Security Policy

Hydra-ESP is a security research tool, but that doesn't mean every part of it is immune to bugs. The firmware and web interface can still have security issues.

We're especially interested in vulnerabilities that could compromise the device, expose captured data such as handshakes, PMKIDs, or Evil Twin passwords to unauthorized parties, or allow remote code execution through the web interface.

## Supported Versions

| Version         | Support       |
| --------------- | ------------- |
| `main` (latest) | Supported     |
| Older releases  | Not supported |

Security fixes are developed for the latest code on `main` and the latest tagged release. Older versions may not receive fixes, so please update before reporting an issue.

## Reporting a Vulnerability

**Please don't report security vulnerabilities through public Issues.** Someone could use the details before a fix is available.

Instead, submit a private report through GitHub:

1. Open the repository's **Security** tab.
2. Select **Report a vulnerability**.
3. Include the affected version or commit, what the issue does, and the steps needed to reproduce it.

If private reporting isn't available, open an Issue asking a maintainer to contact you privately. Don't include exploit code or sensitive details in the public post.

You should receive an initial response within a few days. We'll keep you informed as the report is investigated and, if necessary, a fix is prepared.

Contributors who report valid vulnerabilities will be credited in the release notes unless they prefer to remain anonymous.

## What's In Scope?

Hydra-ESP is designed for security research, so its intended features aren't vulnerabilities by themselves. For example, deauthentication and SSID spoofing are expected behaviors, not security bugs. Please see the Warning section in the README for details.

We're interested in problems that go beyond the tool's intended functionality, including:

* Captured data being exposed to someone other than the device operator.
* Authentication bypasses in the web interface or Settings page.
* Remote code execution or memory corruption caused by untrusted input.
* Other bugs that could compromise the device, its data, or its operator.

If you're unsure whether something qualifies, report it privately anyway. It's better to have a look at a potential issue than to overlook a real vulnerability.
