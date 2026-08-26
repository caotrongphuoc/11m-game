# AK documentation MCP deployment

The Eleven Meter firmware uses the AK documentation MCP server as the source of truth for kernel APIs, guardrails, implementation guides, UART log analysis, and OLED framebuffer decoding.

Upstream repository: <https://github.com/the-ak-foundation/mcp-docs-server>

Project endpoint: `https://ak-mcp.ntbinh-regis.workers.dev/mcp`

## Requirements

- Node.js 20 or newer
- npm
- A Cloudflare account with Workers enabled
- Wrangler authenticated with `npx wrangler login`, or a deployment API token configured for CI

The server vendors its AK public-header snapshot and bundles its generated corpus into the Worker. It does not require a database or a separate firmware checkout at runtime.

## Deploy from a workstation

```bash
git clone https://github.com/the-ak-foundation/mcp-docs-server.git
cd mcp-docs-server
npm install
npm run build:corpus
npm run drift
npm test
npm run typecheck
npm run deploy
```

The upstream `npm run deploy` command rebuilds the corpus and invokes `wrangler deploy`. Confirm that the deployed Worker name and Cloudflare account produce the expected host. To preserve this project's endpoint, configure the Worker as `ak-mcp` in the `ntbinh-regis` Workers subdomain before deploying.

## Verify the endpoint

`/mcp` is a Streamable HTTP MCP endpoint, not a normal browser page. Opening it with a browser sends `GET`, so a JSON-RPC `Method not allowed` response is expected and does not mean the deployment failed.

Verify through an MCP client or the MCP Inspector:

```bash
npx @modelcontextprotocol/inspector
```

Connect the inspector to:

```text
https://ak-mcp.ntbinh-regis.workers.dev/mcp
```

Then call `get_ak_guardrails` or `search_ak_docs`. A successful tool response verifies transport, initialization, and corpus availability.

## Client configuration

For clients that accept remote MCP servers, use:

```json
{
  "mcpServers": {
    "ak-docs": {
      "url": "https://ak-mcp.ntbinh-regis.workers.dev/mcp"
    }
  }
}
```

Restart the client after changing its MCP configuration. The exact configuration file location depends on the client; the upstream repository provides examples for VS Code, Claude Desktop, Cursor, Cline, Claude Code, and Codex.

## Update the AK header snapshot

When AK publishes a new firmware release, refresh the vendored headers and rebuild the corpus before deployment:

```bash
npm run fetch-headers v1.4
npm run build:corpus
npm run drift
npm test
npm run typecheck
npm run deploy
```

Replace `v1.4` with the intended release tag. Review and commit the updated `vendor/ak-inc/` snapshot together with the generated corpus changes.

## CI deployment

The upstream GitHub Actions workflow can deploy changes from `main`. Configure these repository secrets:

- `CLOUDFLARE_API_TOKEN`
- `CLOUDFLARE_ACCOUNT_ID`

The verification job builds the corpus, checks documentation drift, runs tests, and type-checks the server before deployment. Keep those checks passing locally to avoid publishing an incomplete corpus.
