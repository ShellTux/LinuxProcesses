vim.api.nvim_create_autocmd({ "BufWritePost" }, {
	pattern = { "*.c", "*.h" },
	callback = function()
		vim.cmd([[make]])
	end
})
