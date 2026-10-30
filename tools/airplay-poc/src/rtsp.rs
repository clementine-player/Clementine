//! The RTSP (and HTTP) connection to the device, which AirPlay 2 encrypts
//! once pairing is done.

use std::io::{Read, Write};
use std::net::{IpAddr, TcpStream};
use std::time::Duration;

use anyhow::{anyhow, bail, Context, Result};
use log::debug;
use rand::Rng;

use crate::hap::Cipher;

pub const USER_AGENT: &str = "AirPlay/550.10";

pub struct Response {
    pub code: u16,
    pub headers: Vec<(String, String)>,
    pub body: Vec<u8>,
}

impl Response {
    pub fn header(&self, name: &str) -> Option<&str> {
        self.headers
            .iter()
            .find(|(k, _)| k.eq_ignore_ascii_case(name))
            .map(|(_, v)| v.as_str())
    }
}

pub struct Connection {
    stream: TcpStream,
    cipher: Option<Cipher>,
    buffer: Vec<u8>,
    cseq: u32,
    pub session_id: u32,
    dacp_id: String,
    active_remote: u32,
    pub local_ip: IpAddr,
    pub remote_ip: IpAddr,
}

impl Connection {
    pub fn connect(host: IpAddr, port: u16) -> Result<Self> {
        let stream = TcpStream::connect_timeout(&(host, port).into(), Duration::from_secs(5))
            .with_context(|| format!("connecting to {host}:{port}"))?;
        stream.set_read_timeout(Some(Duration::from_secs(10)))?;
        stream.set_nodelay(true)?;
        let mut rng = rand::thread_rng();
        Ok(Connection {
            local_ip: stream.local_addr()?.ip(),
            remote_ip: host,
            stream,
            cipher: None,
            buffer: Vec::new(),
            cseq: 0,
            session_id: rng.gen(),
            dacp_id: format!("{:X}", rng.gen::<u64>()),
            active_remote: rng.gen(),
        })
    }

    pub fn enable_encryption(&mut self, cipher: Cipher) {
        self.cipher = Some(cipher);
    }

    pub fn uri(&self) -> String {
        format!("rtsp://{}/{}", self.local_ip, self.session_id)
    }

    /// Sends a request and waits for its response. An error response is
    /// returned, not raised, so callers can decide.
    pub fn request(
        &mut self,
        method: &str,
        uri: Option<&str>,
        headers: &[(&str, String)],
        content_type: Option<&str>,
        body: &[u8],
        protocol: &str,
    ) -> Result<Response> {
        let uri = uri.map(str::to_owned).unwrap_or_else(|| self.uri());
        let cseq = self.cseq;
        self.cseq += 1;

        let mut msg = format!("{method} {uri} {protocol}\r\n");
        msg += &format!("CSeq: {cseq}\r\n");
        msg += &format!("User-Agent: {USER_AGENT}\r\n");
        msg += &format!("DACP-ID: {}\r\n", self.dacp_id);
        msg += &format!("Active-Remote: {}\r\n", self.active_remote);
        msg += &format!("Client-Instance: {}\r\n", self.dacp_id);
        for (k, v) in headers {
            msg += &format!("{k}: {v}\r\n");
        }
        if let Some(ct) = content_type {
            msg += &format!("Content-Type: {ct}\r\n");
        }
        if !body.is_empty() {
            msg += &format!("Content-Length: {}\r\n", body.len());
        }
        msg += "\r\n";
        debug!(">> {}", msg.trim_end().replace("\r\n", " | "));

        let mut raw = msg.into_bytes();
        raw.extend_from_slice(body);
        let raw = match &mut self.cipher {
            Some(c) => c.encrypt(&raw),
            None => raw,
        };
        self.stream.write_all(&raw)?;

        let resp = self.read_response()?;
        debug!(
            "<< {} {:?} ({} byte body)",
            resp.code,
            resp.headers,
            resp.body.len()
        );
        Ok(resp)
    }

    pub fn rtsp(
        &mut self,
        method: &str,
        headers: &[(&str, String)],
        content_type: Option<&str>,
        body: &[u8],
    ) -> Result<Response> {
        self.request(method, None, headers, content_type, body, "RTSP/1.0")
    }

    /// An RTSP request that has to succeed.
    pub fn rtsp_ok(
        &mut self,
        method: &str,
        headers: &[(&str, String)],
        content_type: Option<&str>,
        body: &[u8],
    ) -> Result<Response> {
        let resp = self.rtsp(method, headers, content_type, body)?;
        if resp.code != 200 {
            bail!("{method} failed: {} {}", resp.code, String::from_utf8_lossy(&resp.body));
        }
        Ok(resp)
    }

    pub fn post(&mut self, path: &str, headers: &[(&str, String)], body: &[u8]) -> Result<Response> {
        self.request(
            "POST",
            Some(path),
            headers,
            Some("application/octet-stream"),
            body,
            "HTTP/1.1",
        )
    }

    fn fill(&mut self) -> Result<()> {
        let mut buf = [0u8; 4096];
        let n = self.stream.read(&mut buf)?;
        if n == 0 {
            bail!("the device closed the connection");
        }
        let data = match &mut self.cipher {
            Some(c) => c.decrypt(&buf[..n])?,
            None => buf[..n].to_vec(),
        };
        self.buffer.extend_from_slice(&data);
        Ok(())
    }

    fn read_response(&mut self) -> Result<Response> {
        let header_end = loop {
            if let Some(i) = find(&self.buffer, b"\r\n\r\n") {
                break i;
            }
            self.fill()?;
        };
        let head = String::from_utf8_lossy(&self.buffer[..header_end]).into_owned();
        let mut lines = head.split("\r\n");
        let status = lines.next().ok_or_else(|| anyhow!("empty response"))?;
        let code: u16 = status
            .split_whitespace()
            .nth(1)
            .and_then(|c| c.parse().ok())
            .ok_or_else(|| anyhow!("bad status line: {status}"))?;
        let headers: Vec<(String, String)> = lines
            .filter_map(|l| l.split_once(':'))
            .map(|(k, v)| (k.trim().to_owned(), v.trim().to_owned()))
            .collect();
        let len: usize = headers
            .iter()
            .find(|(k, _)| k.eq_ignore_ascii_case("Content-Length"))
            .and_then(|(_, v)| v.parse().ok())
            .unwrap_or(0);
        let start = header_end + 4;
        while self.buffer.len() < start + len {
            self.fill()?;
        }
        let body = self.buffer[start..start + len].to_vec();
        self.buffer.drain(..start + len);
        Ok(Response { code, headers, body })
    }
}

pub fn find(hay: &[u8], needle: &[u8]) -> Option<usize> {
    hay.windows(needle.len()).position(|w| w == needle)
}
